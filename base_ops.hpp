#pragma once

#include <llvm/IR/IRBuilder.h>

namespace MyDSL {

class BaseOps {
protected:
  llvm::IRBuilder<> &builder_;
  llvm::Value *value_;

public:
  BaseOps(llvm::Value *value, llvm::IRBuilder<> &builder)
      : builder_(builder), value_(value) {}

  llvm::Type *getType() const { return value_->getType(); }

  llvm::Value *getValue() const { return value_; }
  operator llvm::Value *() const { return value_; }

  friend std::ostream &operator<<(std::ostream &os, const BaseOps &b) {
    std::string str;
    llvm::raw_string_ostream ros(str);
    ros << *b.value_;
    os << str;
    return os;
  }
};

} // namespace MyDSL
