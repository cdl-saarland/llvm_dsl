#include "fuse_ops.hpp"

#include <algorithm>
#include <llvm/ADT/MapVector.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/FMF.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Passes/PassBuilder.h>

namespace {
llvm::CallInst *isElementwiseOp(llvm::Value &V) {
  if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&V)) {
    if (auto *F = CI->getCalledFunction()) {
      if (F->getName().starts_with("__mydsl_tensor_elementwise"))
        return CI;
    }
  }
  return nullptr;
}

llvm::CallInst *isConvolutionOp(llvm::Value &V) {
  if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&V)) {
    if (auto *F = CI->getCalledFunction()) {
      if (F->getName().starts_with("__mydsl_tensor_conv_2_f"))
        return CI;
    }
  }
  return nullptr;
}

std::string getFusedOpName(llvm::CallInst *Elem, llvm::CallInst *Conv) {
  const auto offset = sizeof("__mydsl_tensor_") - 1;
  const auto offset_end = sizeof("_2_f32") + offset - 1;
  auto ElemName = Elem->getCalledFunction()->getName();
  auto ConvName = Conv->getCalledFunction()->getName();
  return ("__mydsl_fused_tensor_" +
          ElemName.substr(offset, ElemName.size() - offset_end) + "_" +
          ConvName.substr(offset, ConvName.size() - offset_end) + "_2_f32")
      .str();
}

llvm::DenseMap<llvm::CallInst *, llvm::CallInst *>
getFusableTensorOps(llvm::Function &F, llvm::DominatorTree &DT) {
  llvm::DenseMap<llvm::CallInst *, llvm::CallInst *> FuseMap;

  auto check_and_insert = [&](llvm::CallInst *ElemWise, llvm::CallInst *Conv) {
    if (DT.dominates(ElemWise, Conv) &&
        ElemWise->getParent() == Conv->getParent() &&
        std::find_if(ElemWise->getArgOperand(0)->use_begin(),
                     ElemWise->getArgOperand(0)->use_end(), [&](llvm::Use &U) {
                       return U.getUser() != ElemWise && U.getUser() != Conv &&
                              U.getOperandNo() == 0 &&
                              DT.dominates(ElemWise, U) &&
                              DT.dominates(U, Conv);
                     }) == ElemWise->getArgOperand(0)->use_end()) {
      // one should also assert that the elem result is only used by conv...
      llvm::errs() << "Fusable tensor op: " << *ElemWise << " with " << *Conv
                   << "\n";
      FuseMap[ElemWise] = Conv;
    }
  };

  // todo: iterate over instructions and find fusable elementwise op &
  // convolution.
  // once you have found a pair, call check_and_insert, which will
  // check legality of the merger.

  return FuseMap;
}

bool fuseElementwiseMulIntoConv(llvm::StringRef FusedOpName, llvm::CallInst *Op,
                                llvm::CallInst *Conv) {
  auto FusedOp = Op->getFunction()->getParent()->getOrInsertFunction(
      FusedOpName, Conv->getType(), Conv->getArgOperand(0)->getType(),
      Op->getArgOperand(1)->getType(), Op->getArgOperand(2)->getType(),
      Conv->getArgOperand(2)->getType(), Conv->getArgOperand(3)->getType(),
      Conv->getArgOperand(4)->getType());

  llvm::IRBuilder<> Builder(Conv);
  auto NewCI = Builder.CreateCall(
      FusedOp,
      {Conv->getArgOperand(0), Op->getArgOperand(1), Op->getArgOperand(2),
       Conv->getArgOperand(2), Conv->getArgOperand(3), Conv->getArgOperand(4)},
      "", Conv->getMetadata(llvm::LLVMContext::MD_fpmath));
  Op->eraseFromParent();
  Conv->replaceAllUsesWith(NewCI);
  Conv->eraseFromParent();
  return true;
}

bool fuseElementwiseOpsIntoConv(llvm::Function &F, llvm::DominatorTree &DT) {
  bool Changed = false;

  auto FuseMap = getFusableTensorOps(F, DT);

  for (auto &[Op, Conv] : FuseMap) {
    auto FusedOpName = getFusedOpName(Op, Conv);
    llvm::errs() << "Fusing " << *Op << " with\n"
                 << *Conv << " to\n"
                 << FusedOpName << "\n";
    if (FusedOpName.starts_with("__mydsl_fused_tensor_elementwise_mul_conv_")) {
      Changed |= fuseElementwiseMulIntoConv(FusedOpName, Op, Conv);
    }
  }

  return Changed;
}

} // namespace

namespace MyDSL {
llvm::PreservedAnalyses
FuseTensorOpsPass::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
  // todo: get DominatorTree from FAM and run pass.

  return llvm::PreservedAnalyses::all();
}

void fuseOps(llvm::Module &M) {
  llvm::PassBuilder PB;

  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  llvm::ModulePassManager MPM;
  // todo: run the pass!
  // alternatively, in jit.cpp register a callback for the PassBuilder
  MPM.run(M, MAM);
}
} // namespace MyDSL
