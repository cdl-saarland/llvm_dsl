#include "control_flow.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"
#include "jit.hpp"
#include "passes/fuse_ops.hpp"
#include "tensor_ops.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

using namespace MyDSL;

void kernel(llvm::Value *Result, llvm::Value *Tensor1, llvm::Value *Tensor2,
            llvm::Value *Size, llvm::IRBuilder<> &Builder) {

  ControlFlow CF(Builder);
  Integer s(Size, Builder);
  llvm::SmallVector<Integer, 2> size{s, s};
  Tensor<Float, 2> T{size, Tensor1, Builder};
  Tensor<Float, 2> T2{size, Tensor2, Builder};

  Integer windowsize{3, Builder};
  llvm::SmallVector<Integer, 2> res_size{s - (windowsize - 1),
                                         s - (windowsize - 1)};
  Tensor<Float, 2> Tres{res_size, Result, Builder};

  llvm::SmallVector<Integer, 2> window{windowsize, windowsize};
  Tensor<Float, 2> filter{window, Builder};

  filter[0][0] = 1.f;
  filter[0][1] = 0.f;
  filter[0][2] = -1.f;
  filter[1][0] = 2.f;
  filter[1][1] = 0.f;
  filter[1][2] = -2.f;
  filter[2][0] = 1.f;
  filter[2][1] = 0.f;
  filter[2][2] = -1.f;

  T *= T2;

  T.conv2d(Tres, filter);

  CF.Return();
}

int main(int argc, const char *argv[]) {

  llvm::ExitOnError ExitOnErr;

  auto [Context, M, JITP] = initialize();
  auto &JIT = *JITP;
  auto &Ctx = *Context;

  auto Kernel = make_kernel(
      M.get(), llvm::Type::getVoidTy(Ctx),
      {Tensor<Float, 2>::getType(Ctx), Tensor<Float, 2>::getType(Ctx),
       Tensor<Float, 2>::getType(Ctx), Integer::getType(Ctx)});

  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  kernel(Kernel->getArg(0), Kernel->getArg(1), Kernel->getArg(2),
         Kernel->getArg(3), Builder);
  llvm::errs() << *Kernel;

  linkBuiltinFunctions(*M);

  optimize(*M, JIT);
  llvm::errs() << "optimized:\n" << *Kernel;
  dumpModule(*M, "kernel.ll");

  const std::size_t size = 10;
  std::vector<Float::NativeType> T1(size * size, 5.f);
  std::vector<Float::NativeType> T2(size * size);
  std::iota(T2.begin(), T2.end(), 0);
  std::transform(T2.begin(), T2.end(), T2.begin(), [](auto I) mutable {
    int i = static_cast<int>(I) % 10;
    return i < 5 ? i : -i;
  });
  std::vector<Float::NativeType> Result(size * size, 1.f);

  void (*FP)(typename Tensor<Float, 2>::NativeType,
             typename Tensor<Float, 2>::NativeType,
             typename Tensor<Float, 2>::NativeType,
             typename Integer::NativeType) =
      (void (*)(typename Tensor<Float, 2>::NativeType,
                typename Tensor<Float, 2>::NativeType,
                typename Tensor<Float, 2>::NativeType,
                typename Integer::NativeType))
          ExitOnErr(JIT(std::move(M), std::move(Context)));

  FP(Result.data(), T1.data(), T2.data(), size);

  for (int i = 0; i < size - 2; ++i) {
    for (int j = 0; j < size - 2; ++j) {
      fprintf(stdout, "%f ", Result[i * (size - 2) + j]);
    }
    fprintf(stdout, "\n");
  }

  return 0;
}
