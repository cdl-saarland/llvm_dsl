#include "bool_ops.hpp"
#include "clangjit.hpp"
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

const std::string module_filename = "kernelsol.ll";
const std::string kernel_fn_name = "kernel";

void dumpModule(llvm::Module &M, llvm::StringRef Filename);
llvm::Function *make_kernel_function(llvm::Module &M, llvm::Type *RetTy,
                                     llvm::ArrayRef<llvm::Type *> ArgTys);

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

  // Generate LLVM IR module and dump to file
  {
    auto Context = std::make_unique<llvm::LLVMContext>();
    auto &Ctx = *Context;
    auto M = std::make_unique<llvm::Module>("top", Ctx);

    auto Kernel = make_kernel_function(
        *M.get(), Float::getType(Ctx),
        {Float::getType(Ctx), Float::getType(Ctx), Float::getType(Ctx)});

    llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

    auto Ret = build_kernel(Kernel->getArg(0), Kernel->getArg(1),
                            Kernel->getArg(2), Builder);
    Builder.CreateRet(Ret);
    llvm::errs() << *Kernel;

    dumpModule(*M, module_filename);
  }

  // JIT compile and execute
  {
    auto Handle = jit::compile(module_filename);
    if(!Handle) {
      std::cerr << "JIT compilation failed\n";
      return 1;
    }
    auto CompiledKernel = jit::getFunctionPtr<float (*)(float, float, float)>(
        Handle, kernel_fn_name);
    if(!CompiledKernel) {
      std::cerr << "Could not get kernel function pointer\n";
      return 1;
    }
    
    auto Eval = CompiledKernel(A, B, C);

    std::cout << "Evaluated to: " << Eval << "\n";
  }

  return 0;
}

llvm::Function *make_kernel_function(llvm::Module &M, llvm::Type *RetTy,
                                     llvm::ArrayRef<llvm::Type *> ArgTys) {
  auto *F = llvm::cast<llvm::Function>(
      M.getOrInsertFunction(kernel_fn_name, RetTy, ArgTys).getCallee());
  llvm::BasicBlock::Create(F->getContext(), "entry", F);
  return F;
}

void dumpModule(llvm::Module &M, llvm::StringRef Filename) {
  std::error_code EC;
  llvm::raw_fd_ostream out(Filename, EC);
  out << M;
}
