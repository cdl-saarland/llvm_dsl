#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>

namespace MyDSL {

class BaseOps {
protected:
  llvm::IRBuilder<> &builder_;
  llvm::AllocaInst *mem_;

  void store(llvm::Value *value) { builder_.CreateStore(value, mem_); }

  llvm::Value *load() const {
    return builder_.CreateLoad(mem_->getAllocatedType(), mem_);
  }

public:
  BaseOps(llvm::Value *value, llvm::IRBuilder<> &builder) : builder_(builder) {
    auto &EntryBB = builder.GetInsertBlock()->getParent()->getEntryBlock();

    mem_ =
        llvm::IRBuilder<>{&EntryBB, EntryBB.getFirstInsertionPt()}.CreateAlloca(
            value->getType());
    if (!llvm::isa<llvm::UndefValue>(value))
      store(value);
  }

  BaseOps(llvm::Type *Ty, llvm::IRBuilder<> &builder)
      : BaseOps(llvm::UndefValue::get(Ty), builder) {}

  BaseOps &operator=(const BaseOps &other) {
    assert(getType() == other.getType() && "Type mismatch");
    store(other.getValue());
    return *this;
  }

  llvm::Type *getType() const { return mem_->getAllocatedType(); }

  llvm::Value *getValue() const { return load(); }
  operator llvm::Value *() const { return getValue(); }

  friend std::ostream &operator<<(std::ostream &os, const BaseOps &b) {
    std::string str;
    llvm::raw_string_ostream ros(str);
    ros << *b.getValue();
    os << str;
    return os;
  }
};

} // namespace MyDSL
