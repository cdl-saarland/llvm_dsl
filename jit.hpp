//===- jit.hpp --------*- C++ -*-===//
//
// Adapted from the Kaleidoscope JIT examples. (Apache-2.0 WITH LLVM-exception)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>

#include <memory>

namespace MyDSL {

class Jit {
private:
  std::unique_ptr<llvm::orc::ExecutionSession> ES;

  llvm::DataLayout DL;
  llvm::orc::MangleAndInterner Mangle;

  llvm::orc::RTDyldObjectLinkingLayer ObjectLayer;
  llvm::orc::IRCompileLayer CompileLayer;

  llvm::orc::JITDylib &MainJD;

public:
  Jit(std::unique_ptr<llvm::orc::ExecutionSession> ES,
      llvm::orc::JITTargetMachineBuilder JTMB, llvm::DataLayout DL)
      : ES(std::move(ES)), DL(std::move(DL)), Mangle(*this->ES, this->DL),
        ObjectLayer(
            *this->ES,
            []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
        CompileLayer(
            *this->ES, ObjectLayer,
            std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(JTMB))),
        MainJD(this->ES->createBareJITDylib("<main>")) {
    MainJD.addGenerator(
        cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix())));
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
  }

  ~Jit() {
    if (auto Err = ES->endSession())
      ES->reportError(std::move(Err));
  }

  static llvm::Expected<std::unique_ptr<Jit>> Create() {
    auto EPC = llvm::orc::SelfExecutorProcessControl::Create();
    if (!EPC)
      return EPC.takeError();

    auto ES = std::make_unique<llvm::orc::ExecutionSession>(std::move(*EPC));

    llvm::orc::JITTargetMachineBuilder JTMB(
        ES->getExecutorProcessControl().getTargetTriple());

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL)
      return DL.takeError();

    return std::make_unique<Jit>(std::move(ES), std::move(JTMB),
                                 std::move(*DL));
  }

  const llvm::DataLayout &getDataLayout() const { return DL; }

  llvm::orc::JITDylib &getMainJITDylib() { return MainJD; }

  llvm::Error addModule(llvm::orc::ThreadSafeModule TSM,
                        llvm::orc::ResourceTrackerSP RT = nullptr) {
    if (!RT)
      RT = MainJD.getDefaultResourceTracker();
    return CompileLayer.add(RT, std::move(TSM));
  }

  llvm::Expected<llvm::JITEvaluatedSymbol> lookup(llvm::StringRef Name) {
    return ES->lookup({&MainJD}, Mangle(Name.str()));
  }
};

} // namespace MyDSL
