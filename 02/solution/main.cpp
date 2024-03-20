#include "bool_ops.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"

#include "jit.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

using namespace MyDSL;

Float kernel(llvm::Value *A, llvm::Value *B, llvm::Value *C,
             llvm::IRBuilder<> &Builder) {
  Float a(A, Builder);
  Float b(B, Builder);
  Float c(C, Builder);

  Float s = (a + b + c) / 2.0f;
  return (s * (s - a) * (s - b) * (s - c)).abs().sqrt();
}

int main(int argc, const char *argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <a> <b> <c>\n";
    return 1;
  }

  using FloatT = Float::NativeType;
  FloatT A = std::stof(argv[1]);
  FloatT B = std::stof(argv[2]);
  FloatT C = std::stof(argv[3]);

  llvm::ExitOnError ExitOnErr;

  auto [Context, M, JITP] = initialize();
  auto &JIT = *JITP;
  auto &Ctx = *Context;

  auto Kernel = make_kernel(
      M.get(), Float::getType(Ctx),
      {Float::getType(Ctx), Float::getType(Ctx), Float::getType(Ctx)});

  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  auto Ret =
      kernel(Kernel->getArg(0), Kernel->getArg(1), Kernel->getArg(2), Builder);
  Builder.CreateRet(Ret);
  llvm::errs() << *Kernel;

  optimize(*M, JIT);
  llvm::errs() << "optimized:\n" << *Kernel;
  dumpModule(*M, "kernel.ll");

  float (*FP)(float, float, float) = (float (*)(float, float, float))ExitOnErr(
      JIT(std::move(M), std::move(Context)));

  std::cout << "Evaluated to " << FP(A, B, C) << std::endl;

  return 0;
}
