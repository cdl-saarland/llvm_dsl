#pragma once

#include "base_ops.hpp"
#include <cmath>
#include <cstdint>
#include <ostream>

#ifdef WITH_JIT
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#endif

#include "bool_ops.hpp"

namespace MyDSL {

class Float;

#ifndef WITH_JIT
class Integer {
  std::uint64_t value_;

public:
  Integer(std::uint64_t value) : value_(value) {}

  Integer operator+(const Integer &other) const {
    return Integer(value_ + other.value_);
  }
  Integer operator-(const Integer &other) const {
    return Integer(value_ - other.value_);
  }
  Integer operator*(const Integer &other) const {
    return Integer(value_ * other.value_);
  }
  Integer operator/(const Integer &other) const {
    return Integer(value_ / other.value_);
  }
  Integer operator-() const { return Integer(-value_); }
  bool operator==(const Integer &other) const { return value_ == other.value_; }
  bool operator!=(const Integer &other) const { return value_ != other.value_; }
  bool operator<(const Integer &other) const { return value_ < other.value_; }
  bool operator<=(const Integer &other) const { return value_ <= other.value_; }
  bool operator>(const Integer &other) const { return value_ > other.value_; }
  bool operator>=(const Integer &other) const { return value_ >= other.value_; }
  Integer &operator+=(const Integer &other) {
    value_ += other.value_;
    return *this;
  }
  Integer &operator-=(const Integer &other) {
    value_ -= other.value_;
    return *this;
  }
  Integer &operator*=(const Integer &other) {
    value_ *= other.value_;
    return *this;
  }
  Integer &operator/=(const Integer &other) {
    value_ /= other.value_;
    return *this;
  }
  Integer &operator++() {
    value_ += 1;
    return *this;
  }
  Integer operator++(int) {
    Integer temp = *this;
    value_ += 1;
    return temp;
  }
  Integer &operator--() {
    value_ -= 1;
    return *this;
  }
  Integer operator--(int) {
    Integer temp = *this;
    value_ -= 1;
    return temp;
  }
  Integer operator%(const Integer &other) const {
    return Integer(fmod(value_, other.value_));
  }
  Integer &operator%=(const Integer &other) {
    value_ = fmod(value_, other.value_);
    return *this;
  }
  Integer operator^(const Integer &other) const {
    return Integer(pow(value_, other.value_));
  }

  std::uint64_t getValue() const { return value_; }

  friend std::ostream &operator<<(std::ostream &os, const Integer &f) {
    os << f.value_;
    return os;
  }
};

#else
class Integer : public BaseOps {
public:
  Integer(std::size_t bits, std::uint64_t value, llvm::IRBuilder<> &builder)
      : BaseOps(llvm::ConstantInt::get(builder.getContext(),
                                       llvm::APInt(bits, value)),
                builder) {}
  Integer(llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(value, builder) {}

  explicit Integer(const BaseOps &base) : BaseOps(base) {
    assert(base.getType()->isIntegerTy());
  }

  /// arithmetic operators
  Integer operator+(const Integer &other) const {
    return {builder_.CreateAdd(value_, other.value_), builder_};
  }
  Integer operator-(const Integer &other) const {
    return {builder_.CreateSub(value_, other.value_), builder_};
  }
  Integer operator*(const Integer &other) const {
    return {builder_.CreateMul(value_, other.value_), builder_};
  }
  Integer operator/(const Integer &other) const {
    // What if we're unsigned? :)
    return {builder_.CreateSDiv(value_, other.value_), builder_};
  }
  Integer operator-() const { return {builder_.CreateNeg(value_), builder_}; }

  /// relational operators
  Bool operator==(const Integer &other) const {
    return {builder_.CreateICmpEQ(value_, other.value_), builder_};
  }
  Bool operator!=(const Integer &other) const {
    return {builder_.CreateICmpNE(value_, other.value_), builder_};
  }
  Bool operator<(const Integer &other) const {
    return {builder_.CreateICmpSLT(value_, other.value_), builder_};
  }
  Bool operator<=(const Integer &other) const {
    return {builder_.CreateICmpSLE(value_, other.value_), builder_};
  }
  Bool operator>(const Integer &other) const {
    return {builder_.CreateICmpSGT(value_, other.value_), builder_};
  }
  Bool operator>=(const Integer &other) const {
    return {builder_.CreateICmpSGE(value_, other.value_), builder_};
  }

  /// compound assignment operators
  Integer &operator+=(const Integer &other) {
    value_ = builder_.CreateAdd(value_, other.value_);
    return *this;
  }
  Integer &operator-=(const Integer &other) {
    value_ = builder_.CreateSub(value_, other.value_);
    return *this;
  }
  Integer &operator*=(const Integer &other) {
    value_ = builder_.CreateMul(value_, other.value_);
    return *this;
  }
  Integer &operator/=(const Integer &other) {
    value_ = builder_.CreateSDiv(value_, other.value_);
    return *this;
  }

  Integer operator%(const Integer &other) const {
    return {builder_.CreateSRem(value_, other.value_), builder_};
  }
  Integer &operator%=(const Integer &other) {
    value_ = builder_.CreateSRem(value_, other.value_);
    return *this;
  }
  Integer operator^(const Integer &other) const {
    return {builder_.CreateCall(llvm::Intrinsic::getDeclaration(
                                    builder_.GetInsertBlock()->getModule(),
                                    llvm::Intrinsic::pow, {value_->getType()}),
                                {value_, other.value_}),
            builder_};
  }

  // operator std::uint64_t() const {
  //   return jit::evaluate<std::uint64_t>(value_);
  // }

  explicit operator Float() const;
};
#endif
} // namespace MyDSL
