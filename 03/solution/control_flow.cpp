#include "control_flow.hpp"

#include "base_ops.hpp"
#include "float_ops.hpp"
#include "int_ops.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace MyDSL {

void ControlFlow::If(const Bool &Cond, const std::function<void()> &ThenTgt,
                     const std::function<void()> &ElseTgt) {
  llvm::Function *F = builder_.GetInsertBlock()->getParent();
  llvm::BasicBlock *Then =
      llvm::BasicBlock::Create(builder_.getContext(), "then", F);
  llvm::BasicBlock *Merge =
      llvm::BasicBlock::Create(builder_.getContext(), "ifcont", F);

  llvm::BasicBlock *Else = Merge;
  if (ElseTgt)
    Else = llvm::BasicBlock::Create(builder_.getContext(), "else", F);

  builder_.CreateCondBr(Cond, Then, Else);

  builder_.SetInsertPoint(Then);
  ThenTgt();
  if (!builder_.GetInsertBlock()->getTerminator())
    builder_.CreateBr(Merge);

  if (ElseTgt) {
    builder_.SetInsertPoint(Else);
    ElseTgt();
    if (!builder_.GetInsertBlock()->getTerminator())
      builder_.CreateBr(Merge);
  }

  builder_.SetInsertPoint(Merge);
}

void ControlFlow::While(const std::function<Bool()> &Cond,
                        const std::function<void()> &Body) {
  llvm::Function *F = builder_.GetInsertBlock()->getParent();
  llvm::BasicBlock *CondBB =
      llvm::BasicBlock::Create(builder_.getContext(), "cond", F);
  llvm::BasicBlock *BodyBB =
      llvm::BasicBlock::Create(builder_.getContext(), "body", F);
  llvm::BasicBlock *AfterBB =
      llvm::BasicBlock::Create(builder_.getContext(), "afterloop", F);

  builder_.CreateBr(CondBB);
  builder_.SetInsertPoint(CondBB);
  builder_.CreateCondBr(Cond(), BodyBB, AfterBB);

  builder_.SetInsertPoint(BodyBB);

  Body();
  const bool BodyHasTerm = builder_.GetInsertBlock()->getTerminator();
  if (!BodyHasTerm)
    builder_.CreateBr(CondBB);

  builder_.SetInsertPoint(AfterBB);
}

void ControlFlow::For(const Integer &Start,
                      const std::function<Bool(const Integer &)> &Cond,
                      const std::function<Integer(const Integer &)> &Step,
                      const std::function<void(const Integer &)> &Body) {
  Integer I{Start};
  While([&]() { return Cond(I); },
        [&]() {
          Body(I);
          I = Step(I);
        });
}

void ControlFlow::Return() { builder_.CreateRetVoid(); }

void ControlFlow::Return(const Float &V) { builder_.CreateRet(V); }
void ControlFlow::Return(const Integer &V) { builder_.CreateRet(V); }
void ControlFlow::Return(const Bool &V) { builder_.CreateRet(V); }
} // namespace MyDSL
