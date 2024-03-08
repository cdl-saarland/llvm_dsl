#pragma once

#include "base_ops.hpp"
#include <cmath>
#include <cstdint>
#include <llvm/IR/Constant.h>
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
  Integer getConst(std::size_t bits, std::uint64_t value) const {
    return {builder_.getIntN(bits, value), builder_};
  }
  Integer(std::size_t bits, std::uint64_t value, llvm::IRBuilder<> &builder)
      : BaseOps(llvm::ConstantInt::get(builder.getContext(),
                                       llvm::APInt(bits, value)),
                builder) {}

public:
  Integer(std::uint64_t value, llvm::IRBuilder<> &builder)
      : BaseOps(llvm::ConstantInt::get(builder.getContext(),
                                       llvm::APInt(64, value)),
                builder) {}

  Integer(llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(value, builder) {}

  explicit Integer(const BaseOps &base) : BaseOps(base) {
    assert(base.getType()->isIntegerTy());
  }

  /// arithmetic operators
  Integer operator+(const Integer &other) const {
    return {builder_.CreateAdd(getValue(), other.getValue()), builder_};
  }
  Integer operator-(const Integer &other) const {
    return {builder_.CreateSub(getValue(), other.getValue()), builder_};
  }
  Integer operator*(const Integer &other) const {
    return {builder_.CreateMul(getValue(), other.getValue()), builder_};
  }
  Integer operator/(const Integer &other) const {
    return {builder_.CreateSDiv(getValue(), other.getValue()), builder_};
  }
  Integer operator-() const {
    return {builder_.CreateNeg(getValue()), builder_};
  }

  Integer operator%(const Integer &other) const {
    return {builder_.CreateSRem(getValue(), other.getValue()), builder_};
  }
  Integer operator^(const Integer &other) const {
    return {
        builder_.CreateCall(llvm::Intrinsic::getDeclaration(
                                builder_.GetInsertBlock()->getModule(),
                                llvm::Intrinsic::pow, {getValue()->getType()}),
                            {getValue(), other.getValue()}),
        builder_};
  }

  Integer operator+(std::uint64_t value) const {
    return *this + getConst(64, value);
  }
  Integer operator-(std::uint64_t value) const {
    return *this - getConst(64, value);
  }
  Integer operator*(std::uint64_t value) const {
    return *this * getConst(64, value);
  }
  Integer operator/(std::uint64_t value) const {
    return *this / getConst(64, value);
  }
  Integer operator%(std::uint64_t value) const {
    return *this % getConst(64, value);
  }
  Integer operator^(std::uint64_t value) const {
    return *this ^ getConst(64, value);
  }

  friend Integer operator+(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) + other;
  }
  friend Integer operator-(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) - other;
  }
  friend Integer operator*(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) * other;
  }
  friend Integer operator/(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) / other;
  }
  friend Integer operator%(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) % other;
  }
  friend Integer operator^(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) ^ other;
  }

  /// relational operators
  Bool operator==(const Integer &other) const {
    return {builder_.CreateICmpEQ(getValue(), other.getValue()), builder_};
  }
  Bool operator!=(const Integer &other) const {
    return {builder_.CreateICmpNE(getValue(), other.getValue()), builder_};
  }
  Bool operator<(const Integer &other) const {
    return {builder_.CreateICmpSLT(getValue(), other.getValue()), builder_};
  }
  Bool operator<=(const Integer &other) const {
    return {builder_.CreateICmpSLE(getValue(), other.getValue()), builder_};
  }
  Bool operator>(const Integer &other) const {
    return {builder_.CreateICmpSGT(getValue(), other.getValue()), builder_};
  }
  Bool operator>=(const Integer &other) const {
    return {builder_.CreateICmpSGE(getValue(), other.getValue()), builder_};
  }

  Bool operator==(std::uint64_t value) const {
    return *this == getConst(64, value);
  }
  Bool operator!=(std::uint64_t value) const {
    return *this != getConst(64, value);
  }
  Bool operator<(std::uint64_t value) const {
    return *this < getConst(64, value);
  }
  Bool operator<=(std::uint64_t value) const {
    return *this <= getConst(64, value);
  }
  Bool operator>(std::uint64_t value) const {
    return *this > getConst(64, value);
  }
  Bool operator>=(std::uint64_t value) const {
    return *this >= getConst(64, value);
  }

  friend Bool operator==(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) == other;
  }
  friend Bool operator!=(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) != other;
  }
  friend Bool operator<(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) < other;
  }
  friend Bool operator<=(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) <= other;
  }
  friend Bool operator>(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) > other;
  }
  friend Bool operator>=(std::uint64_t value, const Integer &other) {
    return other.getConst(64, value) >= other;
  }

  /// compound assignment operators
  Integer &operator+=(const Integer &other) {
    store(builder_.CreateAdd(getValue(), other.getValue()));
    return *this;
  }
  Integer &operator-=(const Integer &other) {
    store(builder_.CreateSub(getValue(), other.getValue()));
    return *this;
  }
  Integer &operator*=(const Integer &other) {
    store(builder_.CreateMul(getValue(), other.getValue()));
    return *this;
  }
  Integer &operator/=(const Integer &other) {
    store(builder_.CreateSDiv(getValue(), other.getValue()));
    return *this;
  }
  Integer &operator%=(const Integer &other) {
    store(builder_.CreateSRem(getValue(), other.getValue()));
    return *this;
  }
  Integer &operator^=(const Integer &other) {
    store(builder_.CreateCall(
        llvm::Intrinsic::getDeclaration(builder_.GetInsertBlock()->getModule(),
                                        llvm::Intrinsic::pow,
                                        {getValue()->getType()}),
        {getValue(), other.getValue()}));
    return *this;
  }

  Integer &operator+=(std::uint64_t value) {
    return *this += getConst(64, value);
  }
  Integer &operator-=(std::uint64_t value) {
    return *this -= getConst(64, value);
  }
  Integer &operator*=(std::uint64_t value) {
    return *this *= getConst(64, value);
  }
  Integer &operator/=(std::uint64_t value) {
    return *this /= getConst(64, value);
  }
  Integer &operator%=(std::uint64_t value) {
    return *this %= getConst(64, value);
  }
  Integer &operator^=(std::uint64_t value) {
    return *this ^= getConst(64, value);
  }

  // operator std::uint64_t() const {
  //   return jit::evaluate<std::uint64_t>(getValue());
  // }

  explicit operator Float() const;
};
#endif
} // namespace MyDSL
