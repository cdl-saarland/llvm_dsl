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

void kernel(llvm::Value *DestTensor, llvm::Value *TensorA, llvm::Value *TensorB,
            llvm::Value *S, llvm::Value *A, llvm::Value *B,
            llvm::IRBuilder<> &Builder) {
  Integer s(S, Builder);
  Tensor<Float, 2> T{{s, s}, TensorA, Builder};
  Tensor<Float, 2> T2{{s, s}, TensorB, Builder};
  Tensor<Float, 2> Dest{{s, s}, DestTensor, Builder};

  T.mmul(Dest, T2);
  ControlFlow CF(Builder);
  CF.Return();
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
      M.get(), llvm::Type::getVoidTy(Ctx),
      {Tensor<Float, 2>::getType(Ctx), Tensor<Float, 2>::getType(Ctx),
       Tensor<Float, 2>::getType(Ctx), Integer::getType(Ctx),
       Float::getType(Ctx), Float::getType(Ctx)});

  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  kernel(Kernel->getArg(0), Kernel->getArg(1), Kernel->getArg(2),
         Kernel->getArg(3), Kernel->getArg(4), Kernel->getArg(5), Builder);
  llvm::errs() << *Kernel;

  linkBuiltinFunctions(*M);

  optimize(*M, JIT);
  llvm::errs() << "optimized:\n" << *Kernel;
  dumpModule(*M, "kernel.ll");

  std::vector<Float::NativeType> T1(Size * Size);
  std::vector<Float::NativeType> T2(Size * Size);
  std::iota(T1.begin(), T1.end(), 1);
  std::transform(T2.begin(), T2.end(), T2.begin(), [Size](auto I) mutable {
    return static_cast<FloatT>(static_cast<int>(I) % Size) / Size;
  });
  std::iota(T2.begin(), T2.end(), 0);
  std::transform(T2.begin(), T2.end(), T2.begin(), [Size](auto I) mutable {
    int i = static_cast<int>(I) % Size;
    return static_cast<FloatT>(i < Size / 2 ? I : -I) / (Size * Size);
  });

  std::vector<Float::NativeType> Dest(Size * Size);

  void (*FP)(typename Tensor<Float, 2>::NativeType,
             typename Tensor<Float, 2>::NativeType,
             typename Tensor<Float, 2>::NativeType, IntT, FloatT, FloatT) =
      ExitOnErr(JIT(std::move(M), std::move(Context))).toPtr<void (typename Tensor<Float, 2>::NativeType,
                typename Tensor<Float, 2>::NativeType,
                typename Tensor<Float, 2>::NativeType, IntT, FloatT,
                FloatT)>();

  FP(Dest.data(), T1.data(), T2.data(), Size, A, B);

  for (int i = 0; i < Size; ++i) {
    for (int j = 0; j < Size; ++j) {
      fprintf(stdout, "%f ", Dest[i * Size + j]);
    }
    fprintf(stdout, "\n");
  }

  return 0;
}
