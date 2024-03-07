#pragma once

#include <cmath>
#include <ostream>

#ifndef WITH_JIT
namespace MyDSL {

class Bool {
  bool value_;

public:
  Bool(bool value) : value_(value) {}

  Bool operator&&(const Bool &other) const {
    return Bool(value_ && other.value_);
  }
  Bool operator||(const Bool &other) const {
    return Bool(value_ || other.value_);
  }
  Bool operator!() const { return Bool(!value_); }
  Bool operator==(const Bool &other) const { return value_ == other.value_; }
  Bool operator!=(const Bool &other) const { return value_ != other.value_; }
  Bool operator<(const Bool &other) const { return value_ < other.value_; }
  Bool operator<=(const Bool &other) const { return value_ <= other.value_; }
  Bool operator>(const Bool &other) const { return value_ > other.value_; }
  Bool operator>=(const Bool &other) const { return value_ >= other.value_; }
  Bool &operator+=(const Bool &other) {
    value_ += other.value_;
    return *this;
  }
  Bool &operator-=(const Bool &other) {
    value_ -= other.value_;
    return *this;
  }
  Bool &operator*=(const Bool &other) {
    value_ *= other.value_;
    return *this;
  }
  Bool &operator/=(const Bool &other) {
    value_ /= other.value_;
    return *this;
  }
  Bool &operator++() {
    value_ += 1;
    return *this;
  }
  Bool operator++(int) {
    Bool temp = *this;
    value_ += 1;
    return temp;
  }
  Bool &operator--() {
    value_ -= 1;
    return *this;
  }
  Bool operator--(int) {
    Bool temp = *this;
    value_ -= 1;
    return temp;
  }
  friend std::ostream &operator<<(std::ostream &os, const Bool &b) {
    os << b.value_;
    return os;
  }
  operator bool() const { return value_; }
  operator int() const { return value_; }
};

#else
#include <llvm/IR/IRBuilder.h>

namespace MyDSL {
class Bool {
  llvm::IRBuilder<> &builder_;
  llvm::Value *value_;

public:
  Bool(bool value, llvm::IRBuilder<> &builder) : builder_(builder) {
    value_ = builder_.getInt1(value);
  }
  Bool(llvm::Value *value, llvm::IRBuilder<> &builder)
      : builder_(builder), value_(value) {}

  // Logical operators
  Bool operator&&(const Bool &other) const {
    return {builder_.CreateAnd(value_, other.value_), builder_};
  }
  Bool operator||(const Bool &other) const {
    return {builder_.CreateOr(value_, other.value_), builder_};
  }
  Bool operator!() const { return {builder_.CreateNot(value_), builder_}; }

  // Relational operators
  Bool operator==(const Bool &other) const {
    return {builder_.CreateICmpEQ(value_, other.value_), builder_};
  }
  Bool operator!=(const Bool &other) const {
    return {builder_.CreateICmpNE(value_, other.value_), builder_};
  }
  Bool operator<(const Bool &other) const {
    return {builder_.CreateICmpULT(value_, other.value_), builder_};
  }
  Bool operator<=(const Bool &other) const {
    return {builder_.CreateICmpULE(value_, other.value_), builder_};
  }
  Bool operator>(const Bool &other) const {
    return {builder_.CreateICmpUGT(value_, other.value_), builder_};
  }
  Bool operator>=(const Bool &other) const {
    return {builder_.CreateICmpUGE(value_, other.value_), builder_};
  }

  friend std::ostream &operator<<(std::ostream &os, const Bool &b) {
    std::string str;
    llvm::raw_string_ostream ros(str);
    ros << *b.value_;
    os << str;
    return os;
  }
  // operator bool() const { return jit::evaluate<bool>(value_); }
  // operator int() const { return jit::evaluate<int>(value_); }

  llvm::Type *getType() const { return value_->getType(); }

  operator llvm::Value *() const { return value_; }
};

#endif
}
