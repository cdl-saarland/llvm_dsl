#include "control_flow.hpp"

#include "float_ops.hpp"
#include "int_ops.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace MyDSL {

void ControlFlow::If(Bool Cond, const std::function<void()> &ThenTgt,
                     const std::function<void()> &ElseTgt) {
  llvm::Function *F = builder_.GetInsertBlock()->getParent();
  llvm::BasicBlock *Then =
      llvm::BasicBlock::Create(builder_.getContext(), "then", F);
  llvm::BasicBlock *Else =
      llvm::BasicBlock::Create(builder_.getContext(), "else", F);
  llvm::BasicBlock *Merge =
      llvm::BasicBlock::Create(builder_.getContext(), "ifcont", F);

  builder_.CreateCondBr(Cond, Then, Else);

  builder_.SetInsertPoint(Then);
  ThenTgt();
  if (!builder_.GetInsertBlock()->getTerminator())
    builder_.CreateBr(Merge);

  builder_.SetInsertPoint(Else);
  ElseTgt();
  if (!builder_.GetInsertBlock()->getTerminator())
    builder_.CreateBr(Merge);

  builder_.SetInsertPoint(Merge);
}

void ControlFlow::Return() {
  builder_.CreateRetVoid();
}

void ControlFlow::Return(Float V) {
  builder_.CreateRet(V);
}
void ControlFlow::Return(Integer V) {
  builder_.CreateRet(V);
}
void ControlFlow::Return(Bool V) {
  builder_.CreateRet(V);
}
} // namespace MyDSL
