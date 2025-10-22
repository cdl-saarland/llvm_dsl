#include "bool_ops.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

using namespace MyDSL;

void dumpModule(llvm::Module &M, llvm::StringRef Filename);
llvm::Function *make_kernel(llvm::Module &M, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Type *> ArgTys);

Float kernel(llvm::Value *A, llvm::Value *B, llvm::Value *C,
             llvm::IRBuilder<> &Builder) {
  Float a(A, Builder);
  Float b(B, Builder);
  Float c(C, Builder);

  // todo: implement me!

  return a;
}

int main(int argc, const char *argv[]) {
  using FloatT = Float::NativeType;
  auto Context = std::make_unique<llvm::LLVMContext>();
  auto &Ctx = *Context;
  auto M = std::make_unique<llvm::Module>("top", Ctx);

  auto Kernel = make_kernel(
      *M.get(), Float::getType(Ctx),
      {Float::getType(Ctx), Float::getType(Ctx), Float::getType(Ctx)});

  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  auto Ret =
      kernel(Kernel->getArg(0), Kernel->getArg(1), Kernel->getArg(2), Builder);
  Builder.CreateRet(Ret);
  llvm::errs() << *Kernel;

  dumpModule(*M, "kernel.ll");

  return 0;
}

llvm::Function *make_kernel(llvm::Module &M, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Type *> ArgTys) {
  auto *F = llvm::cast<llvm::Function>(
      M.getOrInsertFunction("kernel", RetTy, ArgTys).getCallee());
  llvm::BasicBlock::Create(F->getContext(), "entry", F);
  return F;
}

void dumpModule(llvm::Module &M, llvm::StringRef Filename) {
  std::error_code EC;
  llvm::raw_fd_ostream out(Filename, EC);
  out << M;
}
