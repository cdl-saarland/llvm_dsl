#include "control_flow.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"
#include "jit.hpp"

#include <iostream>
#include <memory>

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/FMF.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Scalar/LoopPassManager.h>

using namespace MyDSL;

Float kernel(llvm::Value *A, llvm::Value *B, llvm::IRBuilder<> &Builder) {
  Float a(A, Builder);
  Float b(B, Builder);

  ControlFlow CF(Builder);
  return CF.If<Float>(
      a < b,
      [&]() {
        return static_cast<Float>(static_cast<Integer>(a + b) -
                                  static_cast<Integer>(a + b));
      },
      [&]() { return (a + b) - (a + b); });
}

llvm::Function *make_kernel(llvm::Module *M, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Type *> ArgTys) {
  auto *F = llvm::cast<llvm::Function>(
      M->getOrInsertFunction("kernel", RetTy, ArgTys).getCallee());
  llvm::BasicBlock::Create(F->getContext(), "entry", F);
  return F;
}

void optimize(llvm::Module *M) {
  llvm::PipelineTuningOptions PTO;
  PTO.SLPVectorization = true;
  llvm::PassBuilder PB(nullptr, PTO);

  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  auto MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
  MPM.run(*M, MAM);
}

int main(int argc, const char *argv[]) {
  llvm::ExitOnError ExitOnErr;

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto Context = std::make_unique<llvm::LLVMContext>();
  auto &Ctx = *Context;
  auto M = std::make_unique<llvm::Module>("top", Ctx);

  auto Kernel =
      make_kernel(M.get(), llvm::Type::getFloatTy(Ctx),
                  {llvm::Type::getFloatTy(Ctx), llvm::Type::getFloatTy(Ctx)});
  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  llvm::FastMathFlags FMF;
  FMF.setFast();
  // FMF.setAllowReassoc();
  // FMF.setAllowContract();
  // FMF.setAllowReciprocal();
  // FMF.setNoInfs();
  // FMF.setNoNaNs();
  // FMF.setNoSignedZeros();

  Builder.setFastMathFlags(FMF);

  auto result = kernel(Kernel->getArg(0), Kernel->getArg(1), Builder);

  Builder.CreateRet(result);
  llvm::errs() << *Kernel;

  auto JIT = ExitOnErr(Jit::Create());

  M->setDataLayout(JIT->getDataLayout());

  optimize(M.get());
  llvm::errs() << "optimized:\n" << *Kernel;

  auto RT = JIT->getMainJITDylib().createResourceTracker();

  auto TSM = llvm::orc::ThreadSafeModule(std::move(M), std::move(Context));
  ExitOnErr(JIT->addModule(std::move(TSM), RT));
  auto ExprSymbol = JIT->lookup("kernel").get();

  float (*FP)(float, float) = (float (*)(float, float))ExprSymbol.getAddress();
  fprintf(stderr, "Evaluated to %f\n", FP(5, 2));

  ExitOnErr(RT->remove());

  return 0;
}
