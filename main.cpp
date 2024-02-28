#include "float_ops.hpp"
#include "jit.hpp"

#include <iostream>
#include <memory>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>

using namespace MyDSL;

Float kernel(llvm::Value* A, llvm::Value* B, llvm::IRBuilder<>& Builder) {
  Float a(A, Builder);
  Float b(B, Builder);
  return a + b - a * b / a;
}

int main(int argc, const char *argv[]) {
  llvm::ExitOnError ExitOnErr;

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto Context = std::make_unique<llvm::LLVMContext>();
  auto &Ctx = *Context;
  auto M = std::make_unique<llvm::Module>("top", Ctx);

  llvm::Function *Kernel = llvm::cast<llvm::Function>(
      M->getOrInsertFunction("kernel", llvm::Type::getFloatTy(Ctx),
                             llvm::Type::getFloatTy(Ctx),
                             llvm::Type::getFloatTy(Ctx))
          .getCallee());

  llvm::BasicBlock *Entry = llvm::BasicBlock::Create(Ctx, "entry", Kernel);
  llvm::IRBuilder<> Builder(Entry);

  auto result = kernel(Kernel->arg_begin(), Kernel->arg_begin() + 1, Builder);

  Builder.CreateRet(result);
  llvm::errs() << *Kernel;

  auto JIT = ExitOnErr(llvm::orc::KaleidoscopeJIT::Create());

  M->setDataLayout(JIT->getDataLayout());

  auto RT = JIT->getMainJITDylib().createResourceTracker();

  auto TSM = llvm::orc::ThreadSafeModule(std::move(M), std::move(Context));
  ExitOnErr(JIT->addModule(std::move(TSM), RT));
  auto ExprSymbol = JIT->lookup("kernel").get();

  float (*FP)(float, float) = (float (*)(float, float))ExprSymbol.getAddress();
  fprintf(stderr, "Evaluated to %f\n", FP(5, 2));

  // Delete the anonymous expression module from the JIT.
  ExitOnErr(RT->remove());

  return 0;
}
