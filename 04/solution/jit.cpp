#include "jit.hpp"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

namespace MyDSL {

std::tuple<std::unique_ptr<llvm::LLVMContext>, std::unique_ptr<llvm::Module>,
           std::unique_ptr<Jit>>
initialize() {
  llvm::ExitOnError ExitOnErr;

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto Context = std::make_unique<llvm::LLVMContext>();
  auto &Ctx = *Context;
  auto M = std::make_unique<llvm::Module>("top", Ctx);

  auto JIT = ExitOnErr(Jit::Create());

  M->setDataLayout(JIT->getDataLayout());

  return {std::move(Context), std::move(M), std::move(JIT)};
}

llvm::Function *make_kernel(llvm::Module *M, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Type *> ArgTys) {
  auto *F = llvm::cast<llvm::Function>(
      M->getOrInsertFunction("kernel", RetTy, ArgTys).getCallee());
  llvm::BasicBlock::Create(F->getContext(), "entry", F);
  return F;
}

void optimize(llvm::Module &M, Jit &JIT) {
  llvm::ExitOnError ExitOnErr;
  llvm::PipelineTuningOptions PTO;
  PTO.SLPVectorization = true;
  // disabled for readability, enable for full optimization
  PTO.LoopVectorization = false;
  PTO.LoopUnrolling = false;

  auto TM = ExitOnErr(JIT.getTargetMachine());

  // Create a pass builder that is tailored to the host device
  llvm::PassBuilder PB(TM.get(), PTO);

  // Boilerplate.. just adding all the available analyses
  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  // full optimization pipeline
  auto MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);

  // if instead readability is the main goal, just use the following:
  // llvm::ModulePassManager MPM;
  // llvm::FunctionPassManager FPM;
  // FPM.addPass(llvm::PromotePass());
  // MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

  MPM.run(M, MAM);
}

void dumpModule(llvm::Module &M, llvm::StringRef Filename) {
  std::error_code EC;
  llvm::raw_fd_ostream out(Filename, EC);
  out << M;
}

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
  auto LibBufferE = llvm::MemoryBuffer::getFile("solution/lib/libbuiltin-host-full-sol.bc",
                                                -1, false, true);
  if (!LibBufferE) {
    llvm::errs() << "Error loading libbuiltin-host-full.bc\n" << LibBufferE.getError().message() << "\n";
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
