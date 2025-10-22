#include "jit.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

namespace MyDSL {

std::tuple<std::unique_ptr<llvm::LLVMContext>, std::unique_ptr<llvm::Module>,
           std::unique_ptr<Jit>>
initialize() {
  llvm::ExitOnError ExitOnErr;

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto Context = std::make_unique<llvm::LLVMContext>();
  auto &Ctx = *Context;
  auto M = std::make_unique<llvm::Module>("top", Ctx);

  auto JIT = ExitOnErr(Jit::Create());

  M->setDataLayout(JIT->getDataLayout());

  return {std::move(Context), std::move(M), std::move(JIT)};
}

llvm::Function *make_kernel_function(llvm::Module *M, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Type *> ArgTys) {
  auto *F = llvm::cast<llvm::Function>(
      M->getOrInsertFunction("kernel", RetTy, ArgTys).getCallee());
  llvm::BasicBlock::Create(F->getContext(), "entry", F);
  return F;
}

void optimize(llvm::Module &M, Jit &JIT) {
  llvm::ExitOnError ExitOnErr;
  llvm::PipelineTuningOptions PTO;
  PTO.SLPVectorization = true;
  // disabled for readability, enable for full optimization
  PTO.LoopVectorization = false;
  PTO.LoopUnrolling = false;

  auto TM = ExitOnErr(JIT.getTargetMachine());

  // Create a pass builder that is tailored to the host device
  llvm::PassBuilder PB(TM.get(), PTO);

  // Boilerplate.. just adding all the available analyses
  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  // full optimization pipeline
  auto MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

  // if instead readability is the main goal, just use the following:
  // llvm::ModulePassManager MPM;
  // llvm::FunctionPassManager FPM;
  // FPM.addPass(llvm::PromotePass());
  // MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

  MPM.run(M, MAM);
}

void dumpModule(llvm::Module &M, llvm::StringRef Filename) {
  std::error_code EC;
  llvm::raw_fd_ostream out(Filename, EC);
  out << M;
}

} // namespace MyDSL
