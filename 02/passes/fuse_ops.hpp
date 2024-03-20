#pragma once

#include <llvm/IR/PassManager.h>

namespace MyDSL {

struct FuseTensorOpsPass : llvm::PassInfoMixin<FuseTensorOpsPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM);
};

} // namespace MyDSL
