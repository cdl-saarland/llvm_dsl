#include "fuse_ops.hpp"

#include <algorithm>
#include <llvm/ADT/MapVector.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/FMF.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>

namespace {
llvm::CallInst *isElementwiseOp(llvm::Value &V) {
  if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&V)) {
    if (auto *F = CI->getCalledFunction()) {
      if (F->getName().startswith("__mydsl_tensor_elementwise"))
        return CI;
    }
  }
  return nullptr;
}

llvm::CallInst *isConvolutionOp(llvm::Value &V) {
  if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&V)) {
    if (auto *F = CI->getCalledFunction()) {
      if (F->getName().startswith("__mydsl_tensor_conv_2_f"))
        return CI;
    }
  }
  return nullptr;
}

llvm::CallInst *isTensorOp(llvm::Value &V) {
  if (auto *CI = llvm::dyn_cast<llvm::CallInst>(&V)) {
    if (auto *F = CI->getCalledFunction()) {
      if (F->getName().startswith("__mydsl_tensor"))
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
  for (auto &BB : F) {
    for (auto &I : BB) {
      if (auto *CI = isElementwiseOp(I)) {
        for (auto &U : CI->getArgOperand(0)->uses()) {
          if (auto Conv = isConvolutionOp(*U.getUser())) {
            if (DT.dominates(CI, Conv) &&
                std::find_if(
                    CI->getArgOperand(0)->use_begin(),
                    CI->getArgOperand(0)->use_end(), [&](llvm::Use &U) {
                      return U.getUser() != CI && U.getUser() != Conv &&
                             U.getOperandNo() == 0 && DT.dominates(CI, U) &&
                             DT.dominates(U, Conv);
                    }) == CI->getArgOperand(0)->use_end()) {
              // todo: assert the elem result is only used by conv
              llvm::errs() << "Fusable tensor op: " << *CI << " with " << *Conv
                           << "\n";
              FuseMap[CI] = Conv;
            }
          }
        }
      }
    }
  }
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
  auto &DT = FAM.getResult<llvm::DominatorTreeAnalysis>(F);

  if (fuseElementwiseOpsIntoConv(F, DT)) {
    return llvm::PreservedAnalyses::none();
  }
  return llvm::PreservedAnalyses::all();
}
} // namespace MyDSL
