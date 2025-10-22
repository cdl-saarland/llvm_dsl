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
#include <llvm/ExecutionEngine/Orc/Shared/ExecutorAddress.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Error.h>
#include <llvm/Target/TargetMachine.h>

#include <memory>

namespace MyDSL {

/// A class to manage the JIT.
class Jit {
private:
  std::unique_ptr<llvm::orc::ExecutionSession> ES;

  llvm::DataLayout DL;
  llvm::orc::MangleAndInterner Mangle;

  llvm::orc::RTDyldObjectLinkingLayer ObjectLayer;
  llvm::orc::IRCompileLayer CompileLayer;

  llvm::orc::JITDylib &MainJD;
  llvm::orc::JITTargetMachineBuilder JTMB;

  llvm::orc::ResourceTrackerSP CurrentRT;

  /// Use #Create to create a new instance.
  Jit(std::unique_ptr<llvm::orc::ExecutionSession> ES,
      llvm::orc::JITTargetMachineBuilder JTMB, llvm::DataLayout DL)
      : ES(std::move(ES)), DL(std::move(DL)), Mangle(*this->ES, this->DL),
        ObjectLayer(
            *this->ES,
            []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
        CompileLayer(*this->ES, ObjectLayer,
                     std::make_unique<llvm::orc::ConcurrentIRCompiler>(JTMB)),
        MainJD(this->ES->createBareJITDylib("<main>")), JTMB(std::move(JTMB)) {
    MainJD.addGenerator(
        cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix())));
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
  }

public:
  ~Jit() {
    if (auto Err = CurrentRT->remove())
      ES->reportError(std::move(Err));
    if (auto Err = ES->endSession())
      ES->reportError(std::move(Err));
  }

  /// Creates a new instance of Jit.
  static llvm::Expected<std::unique_ptr<Jit>> Create() {
    auto EPC = llvm::orc::SelfExecutorProcessControl::Create();
    if (!EPC)
      return EPC.takeError();

    auto ES = std::make_unique<llvm::orc::ExecutionSession>(std::move(*EPC));

    auto JTMBC = llvm::orc::JITTargetMachineBuilder::detectHost();
    if (!JTMBC)
      return JTMBC.takeError();
    auto JTMB = std::move(*JTMBC);

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL)
      return DL.takeError();

    return std::unique_ptr<Jit>(
        new Jit(std::move(ES), std::move(JTMB), std::move(*DL)));
  }

  /// Returns the selected data layout.
  const llvm::DataLayout &getDataLayout() const { return DL; }

  /// Returns the selected target machine.
  llvm::Expected<std::unique_ptr<llvm::TargetMachine>> getTargetMachine() {
    return JTMB.createTargetMachine();
  }

  /// Returns a handle to the shared library containing the JITed code.
  llvm::orc::JITDylib &getMainJITDylib() { return MainJD; }

  /// Adds a module to the JIT.
  /// Needs to be a ThreadSafeModule and also takes an ResouceTrackerSP
  /// from the JITDylib.
  llvm::Error addModule(llvm::orc::ThreadSafeModule TSM,
                        llvm::orc::ResourceTrackerSP RT = nullptr) {
    if (!RT)
      RT = MainJD.getDefaultResourceTracker();
    return CompileLayer.add(RT, std::move(TSM));
  }

  /// Looks up a symbol in the JITed shared library.
  /// Use ->getAddress() to get the address, then cast the address to the
  /// function pointer type to use it.
  llvm::Expected<llvm::orc::ExecutorSymbolDef> lookup(llvm::StringRef Name) {
    return ES->lookup({&MainJD}, Mangle(Name.str()));
  }

  /// JIT compiles the code and returns the address of the kernel function.
  llvm::Expected<llvm::orc::ExecutorAddr>
  operator()(std::unique_ptr<llvm::Module> M,
             std::unique_ptr<llvm::LLVMContext> Ctx,
             llvm::StringRef KernelName = "kernel") {

    // Todo: implement
    // First create a ThreadSafeModule from M and Ctx
    // Then add the module to the JIT
    // Finally lookup the kernel function and return its address
    return {llvm::make_error<llvm::StringError>(
        "Not implemented", llvm::inconvertibleErrorCode())};
  }
};

// Initialize and get the LLVM context and module.
std::tuple<std::unique_ptr<llvm::LLVMContext>, std::unique_ptr<llvm::Module>,
           std::unique_ptr<Jit>>
initialize();

// Create a kernel function.
llvm::Function *make_kernel_function(llvm::Module &M, llvm::StringRef Name,
                                     llvm::Type *RetTy,
                                     llvm::ArrayRef<llvm::Type *> ArgTys);

// Optimize the module.
void optimize(llvm::Module &M, Jit &JIT);

// Dump the module to a file.
void dumpModule(llvm::Module &M, llvm::StringRef Filename);

} // namespace MyDSL
