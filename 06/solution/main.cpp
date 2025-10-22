#include "bool_ops.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"
#include "jit.hpp"

#include <iostream>
#include <memory>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

using namespace MyDSL;

const std::string module_filename = "kernelsol.ll";
const std::string kernel_fn_name = "kernel";

Float build_kernel(llvm::Value *A, llvm::Value *B, llvm::Value *C,
                   llvm::IRBuilder<> &Builder) {
  Float a(A, Builder);
  Float b(B, Builder);
  Float c(C, Builder);

  Float s = (a + b + c) / 2.0f;
  return (s * (s - a) * (s - b) * (s - c)).abs().sqrt();
}

// extern "C" float kernel(float a, float b, float c);

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

  // Populate LLVM IR module
  {
    auto Kernel = make_kernel_function(
        *M.get(), kernel_fn_name, Float::getType(Ctx),
        {Float::getType(Ctx), Float::getType(Ctx), Float::getType(Ctx)});

    llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

    auto Ret = build_kernel(Kernel->getArg(0), Kernel->getArg(1),
                            Kernel->getArg(2), Builder);
    Builder.CreateRet(Ret);
    llvm::errs() << *Kernel;

    optimize(*M, JIT);
    llvm::errs() << "optimized:\n" << *Kernel;
    dumpModule(*M, "kernel.ll");
  }

  // JIT compile and execute
  {
    auto CompiledKernel = ExitOnErr(JIT(std::move(M), std::move(Context)))
                              .toPtr<FloatT (*)(FloatT, FloatT, FloatT)>();

    FloatT Eval = CompiledKernel(A, B, C);
    std::cout << "Evaluated to: " << Eval << "\n";
  }

  return 0;
}
