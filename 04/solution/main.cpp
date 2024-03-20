#include "control_flow.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"
#include "jit.hpp"
#include "tensor_ops.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

using namespace MyDSL;

using FloatT = Float::NativeType;
using IntT = Integer::NativeType;

void kernel(llvm::Value *A, llvm::Value *B, llvm::Value *TensorA,
            llvm::Value *TensorB, llvm::Value *S, llvm::IRBuilder<> &Builder) {
  Float a(A, Builder);
  Float b(B, Builder);
  Integer s(S, Builder);

  Tensor<Float, 2> T{{s, s}, TensorA, Builder};
  Tensor<Float, 2> T2{{s, s}, TensorB, Builder};

  T /= T2 * a + b;

  ControlFlow CF(Builder);
  CF.Return(T[4][4]);
}

int main(int argc, const char *argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <a> <b> <size>\n";
    return 1;
  }

  FloatT A = std::stof(argv[1]);
  FloatT B = std::stof(argv[2]);
  IntT Size = std::stoll(argv[3]);

  llvm::ExitOnError ExitOnErr;

  auto [Context, M, JITP] = initialize();
  auto &JIT = *JITP;
  auto &Ctx = *Context;

  auto Kernel = make_kernel(
      M.get(), Float::getType(Ctx),
      {Float::getType(Ctx), Float::getType(Ctx), Tensor<Float, 2>::getType(Ctx),
       Tensor<Float, 2>::getType(Ctx), Integer::getType(Ctx)});

  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  kernel(Kernel->getArg(0), Kernel->getArg(1), Kernel->getArg(2),
         Kernel->getArg(3), Kernel->getArg(4), Builder);
  llvm::errs() << *Kernel;

  linkBuiltinFunctions(*M);

  optimize(*M, JIT);
  llvm::errs() << "optimized:\n" << *Kernel;
  dumpModule(*M, "kernel.ll");

  std::vector<Float::NativeType> T1(Size * Size, 5.f);
  std::vector<Float::NativeType> T2(Size * Size);
  std::iota(T2.begin(), T2.end(), 0);
  std::transform(T2.begin(), T2.end(), T2.begin(), [Size](auto I) mutable {
    int i = static_cast<int>(I) % Size;
    return i < 5 ? i : -i;
  });

  void (*FP)(FloatT, FloatT, typename Tensor<Float, 2>::NativeType,
             typename Tensor<Float, 2>::NativeType, IntT) =
      (void (*)(FloatT, FloatT, typename Tensor<Float, 2>::NativeType,
                typename Tensor<Float, 2>::NativeType,
                IntT))ExitOnErr(JIT(std::move(M), std::move(Context)));

  FP(A, B, T1.data(), T2.data(), Size);

  for (int i = 0; i < Size - 2; ++i) {
    for (int j = 0; j < Size - 2; ++j) {
      fprintf(stdout, "%f ", T1[i * Size + j]);
    }
    fprintf(stdout, "\n");
  }

  return 0;
}
