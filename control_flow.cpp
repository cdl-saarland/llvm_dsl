#include "control_flow.hpp"

#include "base_ops.hpp"
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

BaseOps ControlFlow::IfImpl(Bool Cond,
                        const std::function<BaseOps()> &Then,
                        const std::function<BaseOps()> &Else) {
  llvm::Function *F = builder_.GetInsertBlock()->getParent();
  llvm::BasicBlock *ThenBB =
      llvm::BasicBlock::Create(builder_.getContext(), "then", F);
  llvm::BasicBlock *ElseBB =
      llvm::BasicBlock::Create(builder_.getContext(), "else", F);
  llvm::BasicBlock *MergeBB =
      llvm::BasicBlock::Create(builder_.getContext(), "ifcont", F);

  builder_.CreateCondBr(Cond, ThenBB, ElseBB);

  builder_.SetInsertPoint(ThenBB);
  auto ThenRet = Then();
  const bool ThenHasTerm = builder_.GetInsertBlock()->getTerminator();
  if (!ThenHasTerm)
    builder_.CreateBr(MergeBB);

  builder_.SetInsertPoint(ElseBB);
  auto ElseRet = Else();
  const bool ElseHasTerm = builder_.GetInsertBlock()->getTerminator();
  if (!ElseHasTerm)
    builder_.CreateBr(MergeBB);

  builder_.SetInsertPoint(MergeBB);

  // this variant requires that at least one branch returns.
  assert(!ThenHasTerm || !ElseHasTerm);

  llvm::PHINode *PN = builder_.CreatePHI(ThenRet.getType(), 2);
  if (!ThenHasTerm)
    PN->addIncoming(ThenRet, ThenBB);
  if (!ElseHasTerm)
    PN->addIncoming(ElseRet, ElseBB);
  return {PN, builder_};
}

void ControlFlow::Return() { builder_.CreateRetVoid(); }

void ControlFlow::Return(Float V) { builder_.CreateRet(V); }
void ControlFlow::Return(Integer V) { builder_.CreateRet(V); }
void ControlFlow::Return(Bool V) { builder_.CreateRet(V); }
} // namespace MyDSL
