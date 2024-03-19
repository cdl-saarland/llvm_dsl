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
  using NativeType = bool;

private:
  Bool getConst(NativeType value) const {
    return {builder_.getInt1(value), builder_};
  }

  /// Ref constructor.
  Bool(llvm::Type *type, llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(type, value, builder) {}

  template <class T> friend class Ref;

public:
  Bool(NativeType value, llvm::IRBuilder<> &builder)
      : BaseOps(builder.getInt1(value), builder) {}
  Bool(llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(value, builder) {}

  explicit Bool(const BaseOps &base) : BaseOps(base) {
    assert(base.getType()->isIntegerTy(1));
  }

  /// Get the type, that all Floats have in our DSL.
  static llvm::Type *getType(llvm::LLVMContext &Ctx) {
    return llvm::Type::getInt1Ty(Ctx);
  }

  /**
   * @brief Assignment operator.
   * Stores the native value as constant to the current alloca.
   *
   * @param other The native value to store.
   * @return BaseOps& this.
   */
  BaseOps &operator=(NativeType other) {
    store(getConst(other));
    return *this;
  }

  // Logical operators
  Bool operator&&(const Bool &other) const {
    return {builder_.CreateAnd(getValue(), other.getValue()), builder_};
  }
  Bool operator||(const Bool &other) const {
    return {builder_.CreateOr(getValue(), other.getValue()), builder_};
  }
  Bool operator!() const { return {builder_.CreateNot(getValue()), builder_}; }

  Bool operator&&(NativeType other) const { return *this && getConst(other); }
  Bool operator||(NativeType other) const { return *this || getConst(other); }
  friend Bool operator&&(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) && rhs;
  }
  friend Bool operator||(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) || rhs;
  }

  // Relational operators
  Bool operator==(const Bool &other) const {
    return {builder_.CreateICmpEQ(getValue(), other.getValue()), builder_};
  }
  Bool operator!=(const Bool &other) const {
    return {builder_.CreateICmpNE(getValue(), other.getValue()), builder_};
  }
  Bool operator<(const Bool &other) const {
    return {builder_.CreateICmpULT(getValue(), other.getValue()), builder_};
  }
  Bool operator<=(const Bool &other) const {
    return {builder_.CreateICmpULE(getValue(), other.getValue()), builder_};
  }
  Bool operator>(const Bool &other) const {
    return {builder_.CreateICmpUGT(getValue(), other.getValue()), builder_};
  }
  Bool operator>=(const Bool &other) const {
    return {builder_.CreateICmpUGE(getValue(), other.getValue()), builder_};
  }

  Bool operator==(NativeType other) const { return *this == getConst(other); }
  Bool operator!=(NativeType other) const { return *this != getConst(other); }
  Bool operator<(NativeType other) const { return *this < getConst(other); }
  Bool operator<=(NativeType other) const { return *this <= getConst(other); }
  Bool operator>(NativeType other) const { return *this > getConst(other); }
  Bool operator>=(NativeType other) const { return *this >= getConst(other); }
  friend Bool operator==(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) == rhs;
  }
  friend Bool operator!=(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) != rhs;
  }
  friend Bool operator<(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) < rhs;
  }
  friend Bool operator<=(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) <= rhs;
  }
  friend Bool operator>(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) > rhs;
  }
  friend Bool operator>=(NativeType lhs, const Bool &rhs) {
    return rhs.getConst(lhs) >= rhs;
  }
};

#endif
}
