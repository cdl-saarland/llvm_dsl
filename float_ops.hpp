#pragma once

#include <cmath>
#include <ostream>

#include "base_ops.hpp"
#include "bool_ops.hpp"

#ifdef WITH_JIT
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#endif

namespace MyDSL {
class Integer;

#ifndef WITH_JIT
class Float {
  float value_;

public:
  Float(float value) : value_(value) {}

  Float operator+(const Float &other) const {
    return Float(value_ + other.value_);
  }
  Float operator-(const Float &other) const {
    return Float(value_ - other.value_);
  }
  Float operator*(const Float &other) const {
    return Float(value_ * other.value_);
  }
  Float operator/(const Float &other) const {
    return Float(value_ / other.value_);
  }
  Float operator-() const { return Float(-value_); }
  bool operator==(const Float &other) const { return value_ == other.value_; }
  bool operator!=(const Float &other) const { return value_ != other.value_; }
  bool operator<(const Float &other) const { return value_ < other.value_; }
  bool operator<=(const Float &other) const { return value_ <= other.value_; }
  bool operator>(const Float &other) const { return value_ > other.value_; }
  bool operator>=(const Float &other) const { return value_ >= other.value_; }
  Float &operator+=(const Float &other) {
    value_ += other.value_;
    return *this;
  }
  Float &operator-=(const Float &other) {
    value_ -= other.value_;
    return *this;
  }
  Float &operator*=(const Float &other) {
    value_ *= other.value_;
    return *this;
  }
  Float &operator/=(const Float &other) {
    value_ /= other.value_;
    return *this;
  }
  Float &operator++() {
    value_ += 1;
    return *this;
  }
  Float operator++(int) {
    Float temp = *this;
    value_ += 1;
    return temp;
  }
  Float &operator--() {
    value_ -= 1;
    return *this;
  }
  Float operator--(int) {
    Float temp = *this;
    value_ -= 1;
    return temp;
  }
  Float operator%(const Float &other) const {
    return Float(fmod(value_, other.value_));
  }
  Float &operator%=(const Float &other) {
    value_ = fmod(value_, other.value_);
    return *this;
  }
  Float operator^(const Float &other) const {
    return Float(pow(value_, other.value_));
  }

  float getValue() const { return value_; }

  friend std::ostream &operator<<(std::ostream &os, const Float &f) {
    os << f.value_;
    return os;
  }
};

#else
class Float : public BaseOps {
  Float getConst(float value) const {
    return {llvm::ConstantFP::get(builder_.getContext(), llvm::APFloat(value)),
            builder_};
  }

public:
  Float(float value, llvm::IRBuilder<> &builder)
      : BaseOps(
            llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(value)),
            builder) {}
  Float(llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(value, builder) {}

  explicit Float(const BaseOps &base) : BaseOps(base) {
    assert(base.getType()->isFloatTy());
  }

  /// arithmetic operators
  Float operator+(const Float &other) const {
    return {builder_.CreateFAdd(getValue(), other.getValue()), builder_};
  }
  Float operator-(const Float &other) const {
    return {builder_.CreateFSub(getValue(), other.getValue()), builder_};
  }
  Float operator*(const Float &other) const {
    return {builder_.CreateFMul(getValue(), other.getValue()), builder_};
  }
  Float operator/(const Float &other) const {
    return {builder_.CreateFDiv(getValue(), other.getValue()), builder_};
  }
  Float operator-() const { return {builder_.CreateFNeg(getValue()), builder_}; }

  Float operator+(float f) const { return *this + getConst(f); }
  Float operator-(float f) const { return *this - getConst(f); }
  Float operator*(float f) const { return *this * getConst(f); }
  Float operator/(float f) const { return *this / getConst(f); }

  /// relational operators
  Bool operator==(const Float &other) const {
    return {builder_.CreateFCmpOEQ(getValue(), other.getValue()), builder_};
  }
  Bool operator!=(const Float &other) const {
    return {builder_.CreateFCmpONE(getValue(), other.getValue()), builder_};
  }
  Bool operator<(const Float &other) const {
    return {builder_.CreateFCmpOLT(getValue(), other.getValue()), builder_};
  }
  Bool operator<=(const Float &other) const {
    return {builder_.CreateFCmpOLE(getValue(), other.getValue()), builder_};
  }
  Bool operator>(const Float &other) const {
    return {builder_.CreateFCmpOGT(getValue(), other.getValue()), builder_};
  }
  Bool operator>=(const Float &other) const {
    return {builder_.CreateFCmpOGE(getValue(), other.getValue()), builder_};
  }

  Bool operator==(float f) const { return *this == getConst(f); }
  Bool operator!=(float f) const { return *this != getConst(f); }
  Bool operator<(float f) const { return *this < getConst(f); }
  Bool operator<=(float f) const { return *this <= getConst(f); }
  Bool operator>(float f) const { return *this > getConst(f); }
  Bool operator>=(float f) const { return *this >= getConst(f); }

  /// compound assignment operators
  Float &operator+=(const Float &other) {
    store(builder_.CreateFAdd(getValue(), other.getValue()));
    return *this;
  }
  Float &operator-=(const Float &other) {
    store(builder_.CreateFSub(getValue(), other.getValue()));
    return *this;
  }
  Float &operator*=(const Float &other) {
    store(builder_.CreateFMul(getValue(), other.getValue()));
    return *this;
  }
  Float &operator/=(const Float &other) {
    store(builder_.CreateFDiv(getValue(), other.getValue()));
    return *this;
  }

  Float &operator+=(float f) { return *this += getConst(f); }
  Float &operator-=(float f) { return *this -= getConst(f); }
  Float &operator*=(float f) { return *this *= getConst(f); }
  Float &operator/=(float f) { return *this /= getConst(f); }
  
  Float operator%(const Float &other) const {
    return {builder_.CreateFRem(getValue(), other.getValue()), builder_};
  }
  
  Float &operator%=(const Float &other) {
    store(builder_.CreateFRem(getValue(), other.getValue()));
    return *this;
  }

  Float &operator%= (float f) { return *this %= getConst(f); }

  Float operator^(const Float &other) const {
    return {builder_.CreateCall(llvm::Intrinsic::getDeclaration(
                                    builder_.GetInsertBlock()->getModule(),
                                    llvm::Intrinsic::pow, {getValue()->getType()}),
                                {getValue(), other.getValue()}),
            builder_};
  }

  Float operator^(float f) const { return *this ^ getConst(f); }
  
  // operator float() const {
  //   return jit::evaluate<float>(getValue());
  // }

  explicit operator Integer() const;
};
#endif
} // namespace MyDSL
