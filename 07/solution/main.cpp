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

FloatT dot_product(FloatT a, FloatT b, const std::size_t size) {
  std::vector<FloatT> A(size, a);
  std::vector<FloatT> B(size, b);
  IntT i = 0;
  std::transform(A.begin(), A.end(), A.begin(),
                 [&i](FloatT x) { return x + i++; });
  i = 0;
  std::transform(B.begin(), B.end(), B.begin(),
                 [&i, size](FloatT x) { return x + size - i++; });

  FloatT acc = 0;
  for (IntT i = 0; i < size; ++i)
    acc += A[i] * B[i];

  return acc;
}

void kernel(llvm::Value *A, llvm::Value *B, llvm::Value *Size,
            llvm::IRBuilder<> &Builder) {
  Float a(A, Builder);
  Float b(B, Builder);
  Integer size(Size, Builder);

  Float acc(0.f, Builder);

  ControlFlow CF(Builder);
  Tensor<Float, 1> ArrayA{{size}, Builder};
  Tensor<Float, 1> ArrayB{{size}, Builder};
  // initialize
  CF.For(
      Integer{0, Builder}, [&](Integer i) { return i < size; },
      [&](Integer i) { return i + 1; },
      [&](Integer i) {
        ArrayA[i] = i.toFloat() + a;
        ArrayB[i] = (size - i).toFloat() + b;
      });

  // perform dot product
  CF.For(
      Integer{0, Builder}, [&](Integer i) { return i < size; },
      [&](Integer i) { return i + 1; },
      [&](Integer i) { acc += ArrayA[i] * ArrayB[i]; });

  CF.Return(acc);
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

  auto Kernel = make_kernel_function(
      M.get(), Float::getType(Ctx),
      {Float::getType(Ctx), Float::getType(Ctx), Integer::getType(Ctx)});

  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  kernel(Kernel->getArg(0), Kernel->getArg(1), Kernel->getArg(2), Builder);
  llvm::errs() << *Kernel;

  optimize(*M, JIT);
  llvm::errs() << "optimized:\n" << *Kernel;
  dumpModule(*M, "kernel.ll");

  FloatT (*FP)(FloatT, FloatT, IntT) =
      ExitOnErr(JIT(std::move(M), std::move(Context)))
          .toPtr<FloatT (*)(FloatT, FloatT, IntT)>();

  std::cout << "Evaluated to " << FP(A, B, Size) << " reference "
            << dot_product(A, B, Size) << std::endl;

  return 0;
}
