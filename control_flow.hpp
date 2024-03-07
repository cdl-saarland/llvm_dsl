#pragma once

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "bool_ops.hpp"

namespace MyDSL {

class Integer;
class Float;

class ControlFlow {
  llvm::IRBuilder<> &builder_;

public:
  ControlFlow(llvm::IRBuilder<> &builder) : builder_(builder) {}

  void If(Bool condition, const std::function<void()> &then_block,
          const std::function<void()> &else_block = nullptr);

  template <class RetT>
  RetT If(Bool Cond, const std::function<RetT()> &ThenTgt,
          const std::function<RetT()> &ElseTgt = nullptr) {
    llvm::Function *F = builder_.GetInsertBlock()->getParent();
    llvm::BasicBlock *Then =
        llvm::BasicBlock::Create(builder_.getContext(), "then", F);
    llvm::BasicBlock *Else =
        llvm::BasicBlock::Create(builder_.getContext(), "else", F);
    llvm::BasicBlock *Merge =
        llvm::BasicBlock::Create(builder_.getContext(), "ifcont", F);

    builder_.CreateCondBr(Cond, Then, Else);

    builder_.SetInsertPoint(Then);
    auto ThenRet = ThenTgt();
    const bool ThenHasTerm = builder_.GetInsertBlock()->getTerminator();
    if (!ThenHasTerm)
      builder_.CreateBr(Merge);

    builder_.SetInsertPoint(Else);
    auto ElseRet = ElseTgt();
    const bool ElseHasTerm = builder_.GetInsertBlock()->getTerminator();
    if (!ElseHasTerm)
      builder_.CreateBr(Merge);

    builder_.SetInsertPoint(Merge);

    // this variant requires that at least one branch returns.
    assert(!ThenHasTerm || !ElseHasTerm);

    llvm::PHINode *PN = builder_.CreatePHI(ThenRet.getType(), 2);
    if (!ThenHasTerm)
      PN->addIncoming(ThenRet, Then);
    if (!ElseHasTerm)
      PN->addIncoming(ElseRet, Else);
    return {PN, builder_};
  }

  void While(const Bool &condition, const std::function<void()> &body);
  void For(const Integer &start, const Bool &condition,
           const std::function<void()> &body,
           const std::function<void()> &step);
  void Break();
  void Continue();
  void Return();
  void Return(Integer value);
  void Return(Float value);
  void Return(Bool value);
};

} // namespace MyDSL
