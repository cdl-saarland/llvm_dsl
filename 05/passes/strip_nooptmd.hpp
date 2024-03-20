
#pragma once

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Transforms/Scalar/LoopPassManager.h>

namespace MyDSL {

struct StripNoOptMetadata : llvm::PassInfoMixin<StripNoOptMetadata> {
  llvm::PreservedAnalyses run(llvm::Loop &L, llvm::LoopAnalysisManager &AM,
                              llvm::LoopStandardAnalysisResults &AR,
                              llvm::LPMUpdater &);
                              
};


void stripNoOptMetadata(llvm::Module &M);

} // namespace MyDSL
