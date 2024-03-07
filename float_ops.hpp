#pragma once

#include <cmath>
#include <ostream>

#include "bool_ops.hpp"

#ifdef WITH_JIT
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
class Float {
  llvm::IRBuilder<> &builder_;
  llvm::Value *value_;

public:
  Float(float value, llvm::IRBuilder<> &builder) : builder_(builder) {
    value_ = llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(value));
  }
  Float(llvm::Value *value, llvm::IRBuilder<> &builder)
      : builder_(builder), value_(value) {}

  /// arithmetic operators
  Float operator+(const Float &other) const {
    return {builder_.CreateFAdd(value_, other.value_), builder_};
  }
  Float operator-(const Float &other) const {
    return {builder_.CreateFSub(value_, other.value_), builder_};
  }
  Float operator*(const Float &other) const {
    return {builder_.CreateFMul(value_, other.value_), builder_};
  }
  Float operator/(const Float &other) const {
    return {builder_.CreateFDiv(value_, other.value_), builder_};
  }
  Float operator-() const { return {builder_.CreateFNeg(value_), builder_}; }

  /// relational operators
  Bool operator==(const Float &other) const {
    return {builder_.CreateFCmpOEQ(value_, other.value_), builder_};
  }
  Bool operator!=(const Float &other) const {
    return {builder_.CreateFCmpONE(value_, other.value_), builder_};
  }
  Bool operator<(const Float &other) const {
    return {builder_.CreateFCmpOLT(value_, other.value_), builder_};
  }
  Bool operator<=(const Float &other) const {
    return {builder_.CreateFCmpOLE(value_, other.value_), builder_};
  }
  Bool operator>(const Float &other) const {
    return {builder_.CreateFCmpOGT(value_, other.value_), builder_};
  }
  Bool operator>=(const Float &other) const {
    return {builder_.CreateFCmpOGE(value_, other.value_), builder_};
  }

  /// compound assignment operators
  Float &operator+=(const Float &other) {
    value_ = builder_.CreateFAdd(value_, other.value_);
    return *this;
  }
  Float &operator-=(const Float &other) {
    value_ = builder_.CreateFSub(value_, other.value_);
    return *this;
  }
  Float &operator*=(const Float &other) {
    value_ = builder_.CreateFMul(value_, other.value_);
    return *this;
  }
  Float &operator/=(const Float &other) {
    value_ = builder_.CreateFDiv(value_, other.value_);
    return *this;
  }

  Float operator%(const Float &other) const {
    return {builder_.CreateFRem(value_, other.value_), builder_};
  }
  Float &operator%=(const Float &other) {
    value_ = builder_.CreateFRem(value_, other.value_);
    return *this;
  }
  Float operator^(const Float &other) const {
    return {builder_.CreateCall(llvm::Intrinsic::getDeclaration(
                                    builder_.GetInsertBlock()->getModule(),
                                    llvm::Intrinsic::pow, {value_->getType()}),
                                {value_, other.value_}),
            builder_};
  }

  // operator float() const {
  //   return jit::evaluate<float>(value_);
  // }

  friend std::ostream &operator<<(std::ostream &os, const Float &f) {
    std::string str;
    llvm::raw_string_ostream ros(str);
    ros << *f.value_;
    os << str;
    return os;
  }

  llvm::Type *getType() const { return value_->getType(); }

  llvm::Value *getValue() const { return value_; }

  operator llvm::Value *() const { return value_; }

  explicit operator Integer() const;
};
#endif
} // namespace MyDSL
