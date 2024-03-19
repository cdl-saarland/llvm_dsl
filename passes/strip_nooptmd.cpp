#include "strip_nooptmd.hpp"

#include <llvm/Passes/PassBuilder.h>

namespace MyDSL {

llvm::PreservedAnalyses
StripNoOptMetadata::run(llvm::Loop &L, llvm::LoopAnalysisManager &AM,
                        llvm::LoopStandardAnalysisResults &AR,
                        llvm::LPMUpdater &) {
  auto &Ctx = L.getHeader()->getContext();
  auto NewID = llvm::makePostTransformationMetadata(
      Ctx, L.getLoopID(), {"llvm.loop.unroll.disable"}, {});
  L.setLoopID(NewID);
  return llvm::PreservedAnalyses::all();
}

void stripNoOptMetadata(llvm::Module &M) {
  auto MPM = llvm::ModulePassManager();
  auto LPM = llvm::LoopPassManager();
  auto FPM = llvm::FunctionPassManager();
  LPM.addPass(StripNoOptMetadata());
  FPM.addPass(llvm::createFunctionToLoopPassAdaptor(std::move(LPM)));
  MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

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

  MPM.run(M, MAM);
}

} // namespace MyDSL
