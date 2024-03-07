#pragma once

#include "base_ops.hpp"

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
class Bool : public BaseOps {
public:
  Bool(bool value, llvm::IRBuilder<> &builder)
      : BaseOps(builder.getInt1(value), builder) {}
  Bool(llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(value, builder) {}

  explicit Bool(const BaseOps &base) : BaseOps(base) {
    assert(base.getType()->isIntegerTy(1));
  }

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

  // operator bool() const { return jit::evaluate<bool>(value_); }
  // operator int() const { return jit::evaluate<int>(value_); }
};

#endif
}
