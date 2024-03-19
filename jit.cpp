#include "jit.hpp"

#include "passes/strip_nooptmd.hpp"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>

namespace MyDSL {

bool linkBitcode(llvm::Module &M, std::unique_ptr<llvm::Module> OtherM,
                 const std::string &ForcedTriple = "",
                 const std::string &ForcedDataLayout = "",
                 llvm::Linker::Flags Flags = llvm::Linker::Flags::None) {
  if (!ForcedTriple.empty())
    OtherM->setTargetTriple(ForcedTriple);
  if (!ForcedDataLayout.empty())
    OtherM->setDataLayout(ForcedDataLayout);

  // Returns true on error
  if (llvm::Linker::linkModules(M, std::move(OtherM), Flags)) {
    return false;
  }
  return true;
}

bool linkBuiltinFunctions(llvm::Module &M) {
  llvm::ExitOnError ExitOnErr;
  auto LibBufferE = llvm::MemoryBuffer::getFile("lib/libbuiltin-host-full.bc",
                                                -1, false, true);
  if (!LibBufferE) {
    llvm::errs() << "Error loading libbuiltin-host-full.bc\n";
    return false;
  }
  auto LibBuffer = std::move(*LibBufferE);
  auto NewM = ExitOnErr(llvm::parseBitcodeFile(*LibBuffer, M.getContext()));
  // remember the names of the functions we are about to link
  std::vector<std::string> NewFunctionNames;
  for (auto &F : *NewM) {
    if (F.isDeclaration()) {
      continue;
    }
    NewFunctionNames.push_back(F.getName().str());
  }
  stripNoOptMetadata(*NewM);

  if (linkBitcode(M, std::move(NewM))) {
    // now set the linkage of the new functions to internal, so that they get
    // cleaned up if no longer needed
    for (const auto &Name : NewFunctionNames) {
      if (auto F = M.getFunction(Name)) {
        F->setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);
      }
    }
    return true;
  }
  return false;
}
} // namespace MyDSL
