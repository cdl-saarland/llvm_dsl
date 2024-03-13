#include "control_flow.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"
#include "jit.hpp"
#include "tensor_ops.hpp"

#include <iostream>
#include <memory>
#include <numeric>

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/FMF.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Scalar/LoopPassManager.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

using namespace MyDSL;

void kernel(llvm::Value *Tensor1, llvm::Value *Tensor2, llvm::Value *Size,
            llvm::IRBuilder<> &Builder) {

  ControlFlow CF(Builder);
  Integer s(Size, Builder);
  llvm::SmallVector<Integer, 2> size{s, s};
  Tensor<Float, 2> T{size, Tensor1, Builder};
  Tensor<Float, 2> T2{size, Tensor2, Builder};

  T *= T2;
  CF.Return(T[4][4]);
}

// void kernel(llvm::Value *A, llvm::Value *B, llvm::Value* S, llvm::IRBuilder<>
// &Builder) {
//   Float a(A, Builder);
//   Float b(B, Builder);
//   Integer s(S, Builder);

//   ControlFlow CF(Builder);
//   Tensor<Float, 2> T{{s, s}, Builder};
//   Tensor<Float, 2> T2{{s, s}, Builder};
//   CF.For(
//       Integer{0, Builder}, [&](Integer i) { return i < s; },
//       [&](Integer i) { return i + 1; },
//       [&](Integer i) {
//         CF.For(
//             Integer{0, Builder}, [&](Integer j) { return j < s; },
//             [&](Integer j) { return j + 1; },
//             [&](Integer j) {
//               T[i][j] = static_cast<Float>(i) + static_cast<Float>(j) + a;
//             });
//       });

//   CF.For(
//       Integer{0, Builder}, [&](Integer i) { return i < s; },
//       [&](Integer i) { return i + 1; },
//       [&](Integer i) {
//         CF.For(
//             Integer{0, Builder}, [&](Integer j) { return j < s; },
//             [&](Integer j) { return j + 1; }, [&](Integer j) { T2[i][j] = b;
//             });
//       });

//   // CF.For(
//   //     Integer{0, Builder}, [&](Integer i) { return i < s; },
//   //     [&](Integer i) { return i + 1; },
//   //     [&](Integer i) {
//   //       CF.For(
//   //           Integer{0, Builder}, [&](Integer j) { return j < s; },
//   //           [&](Integer j) { return j + 1; },
//   //           [&](Integer j) { T[i][j] = T[i][j] * T2[i][j]; });
//   //     });
//   T /= b;
//   CF.Return(T[4][4]);
// }

// void kernel(llvm::Value *A, llvm::Value *B, llvm::IRBuilder<> &Builder) {
//   Float a(A, Builder);
//   Float b(B, Builder);

//   ControlFlow CF(Builder);
//   // return CF.If<Float>(
//   //     a < b,
//   //     [&]() {
//   //       a += b;
//   //       return static_cast<Float>(static_cast<Integer>(a + b) -
//   //                                 static_cast<Integer>(a + b));
//   //     },
//   //     [&]() { return (a + b) - (a + b); });
//   // CF.While([&]() { return a < b; },
//   //                 [&]() {
//   //                   a += 1;
//   //                   b -= 1;
//   //                 });
//   Integer i{0ull, Builder};
//   CF.For(
//       i, [&](Integer i) { return i < 10; }, [&](Integer i) { return i + 1; },
//       [&](Integer i) { a += static_cast<Float>(i); });
//   auto res = a + b + a;
//   CF.Return(res);
// }

llvm::Function *make_kernel(llvm::Module *M, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Type *> ArgTys) {
  auto *F = llvm::cast<llvm::Function>(
      M->getOrInsertFunction("kernel", RetTy, ArgTys).getCallee());
  llvm::BasicBlock::Create(F->getContext(), "entry", F);
  return F;
}

void optimize(llvm::Module &M, llvm::TargetMachine &TM) {
  llvm::ExitOnError ExitOnErr;

  llvm::PipelineTuningOptions PTO;
  PTO.SLPVectorization = true;
  PTO.LoopVectorization = true;
  PTO.LoopUnrolling = true;

  llvm::PassBuilder PB(&TM, PTO);

  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  auto MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
  // llvm::ModulePassManager MPM;
  // llvm::FunctionPassManager FPM;
  // FPM.addPass(llvm::PromotePass());
  // MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
  MPM.run(M, MAM);
}

int main(int argc, const char *argv[]) {
  llvm::ExitOnError ExitOnErr;

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto Context = std::make_unique<llvm::LLVMContext>();
  auto &Ctx = *Context;
  auto M = std::make_unique<llvm::Module>("top", Ctx);

  // auto Kernel =
  //     make_kernel(M.get(), llvm::Type::getFloatTy(Ctx),
  //                 {llvm::Type::getFloatTy(Ctx), llvm::Type::getFloatTy(Ctx),
  //                  llvm::IntegerType::get(Ctx, 64)});
  auto Kernel =
      make_kernel(M.get(), Float::getType(Ctx),
                  {Tensor<Float, 2>::getType(Ctx),
                   Tensor<Float, 2>::getType(Ctx), Integer::getType(Ctx)});
  llvm::IRBuilder<> Builder(&Kernel->getEntryBlock());

  llvm::FastMathFlags FMF;
  FMF.setFast();
  // FMF.setAllowReassoc();
  // FMF.setAllowContract();
  // FMF.setAllowReciprocal();
  // FMF.setNoInfs();
  // FMF.setNoNaNs();
  // FMF.setNoSignedZeros();

  Builder.setFastMathFlags(FMF);

  kernel(Kernel->getArg(0), Kernel->getArg(1), Kernel->getArg(2), Builder);
  llvm::errs() << *Kernel;

  auto JIT = ExitOnErr(Jit::Create());

  M->setDataLayout(JIT->getDataLayout());

  optimize(*M, *ExitOnErr(JIT->getTargetMachine()));
  llvm::errs() << "optimized:\n" << *Kernel;

  auto RT = JIT->getMainJITDylib().createResourceTracker();

  auto TSM = llvm::orc::ThreadSafeModule(std::move(M), std::move(Context));
  ExitOnErr(JIT->addModule(std::move(TSM), RT));
  auto ExprSymbol = JIT->lookup("kernel").get();

  // float (*FP)(float, float, std::uint64_t) =
  //     (float (*)(float, float, std::uint64_t))ExprSymbol.getAddress();
  // fprintf(stderr, "Evaluated to %f\n", FP(2, 5, 10ull));

  const std::size_t size = 100;
  std::vector<Float::NativeType> T1(size * size, 5);
  std::vector<Float::NativeType> T2(size * size);
  std::iota(T2.begin(), T2.end(), 0);

  typename Float::NativeType (*FP)(typename Tensor<Float, 2>::NativeType,
                                   typename Tensor<Float, 2>::NativeType,
                                   typename Integer::NativeType) =
      (typename Float::NativeType(*)(
          typename Tensor<Float, 2>::NativeType,
          typename Tensor<Float, 2>::NativeType,
          typename Integer::NativeType))ExprSymbol.getAddress();
  fprintf(stderr, "Evaluated to %f\n", FP(T1.data(), T2.data(), size));

  ExitOnErr(RT->remove());

  return 0;
}
