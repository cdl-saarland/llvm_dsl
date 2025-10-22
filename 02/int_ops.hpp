#pragma once

#include "base_ops.hpp"
#include <cmath>
#include <cstdint>
#include <ostream>

#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

#include "bool_ops.hpp"

namespace MyDSL {

class Float;

class Integer : public BaseOps {
public:
  using NativeType = std::int64_t;

private:
  Integer getConst(std::size_t bits, NativeType value) const {
    return {builder_.getIntN(bits, value), builder_};
  }
  Integer(std::size_t bits, NativeType value, llvm::IRBuilder<> &builder)
      : BaseOps(llvm::ConstantInt::get(builder.getContext(),
                                       llvm::APInt(bits, value)),
                builder) {}

  /// Ref constructor.
  Integer(llvm::Type *type, llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(type, value, builder) {}

  template <class T> friend class Ref;

public:
  /// Initialize with a constant value.
  Integer(auto value, llvm::IRBuilder<> &builder)
      // disambiguate 0 int literal and nullptr
    requires(std::is_integral_v<decltype(value)> &&
             !std::is_same_v<decltype(NULL), decltype(value)>)
      : BaseOps(llvm::ConstantInt::get(builder.getContext(),
                                       llvm::APInt(64, value)),
                builder) {}

  /// Initialize with an input LLVM value (must be int..)
  Integer(llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(value, builder) {
    assert(value->getType()->isIntegerTy() && "Value must be an integer");
  }

  /// Initialize from another BaseOps object (must be int..)
  explicit Integer(const BaseOps &base) : BaseOps(base) {
    assert(base.getType()->isIntegerTy() && "Value must be an integer");
  }

  /// Get the type, that all Integers have in our DSL.
  static llvm::Type *getType(llvm::LLVMContext &Ctx) {
    return llvm::IntegerType::get(Ctx, 64);
  }

  /**
   * @brief Assignment operator.
   * Stores the native value as constant to the current alloca.
   *
   * @param other The native value to store.
   * @return BaseOps& this.
   */
  BaseOps &operator=(NativeType other) {
    store(getConst(64, other));
    return *this;
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

  Integer operator+(NativeType value) const {
    return *this + getConst(64, value);
  }
  Integer operator-(NativeType value) const {
    return *this - getConst(64, value);
  }
  Integer operator*(NativeType value) const {
    return *this * getConst(64, value);
  }
  Integer operator/(NativeType value) const {
    return *this / getConst(64, value);
  }
  Integer operator%(NativeType value) const {
    return *this % getConst(64, value);
  }

  friend Integer operator+(NativeType value, const Integer &other) {
    return other.getConst(64, value) + other;
  }
  friend Integer operator-(NativeType value, const Integer &other) {
    return other.getConst(64, value) - other;
  }
  friend Integer operator*(NativeType value, const Integer &other) {
    return other.getConst(64, value) * other;
  }
  friend Integer operator/(NativeType value, const Integer &other) {
    return other.getConst(64, value) / other;
  }
  friend Integer operator%(NativeType value, const Integer &other) {
    return other.getConst(64, value) % other;
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

  Bool operator==(NativeType value) const {
    return *this == getConst(64, value);
  }
  Bool operator!=(NativeType value) const {
    return *this != getConst(64, value);
  }
  Bool operator<(NativeType value) const { return *this < getConst(64, value); }
  Bool operator<=(NativeType value) const {
    return *this <= getConst(64, value);
  }
  Bool operator>(NativeType value) const { return *this > getConst(64, value); }
  Bool operator>=(NativeType value) const {
    return *this >= getConst(64, value);
  }

  friend Bool operator==(NativeType value, const Integer &other) {
    return other.getConst(64, value) == other;
  }
  friend Bool operator!=(NativeType value, const Integer &other) {
    return other.getConst(64, value) != other;
  }
  friend Bool operator<(NativeType value, const Integer &other) {
    return other.getConst(64, value) < other;
  }
  friend Bool operator<=(NativeType value, const Integer &other) {
    return other.getConst(64, value) <= other;
  }
  friend Bool operator>(NativeType value, const Integer &other) {
    return other.getConst(64, value) > other;
  }
  friend Bool operator>=(NativeType value, const Integer &other) {
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

  Integer &operator+=(NativeType value) { return *this += getConst(64, value); }
  Integer &operator-=(NativeType value) { return *this -= getConst(64, value); }
  Integer &operator*=(NativeType value) { return *this *= getConst(64, value); }
  Integer &operator/=(NativeType value) { return *this /= getConst(64, value); }
  Integer &operator%=(NativeType value) { return *this %= getConst(64, value); }

  Integer abs() const {
    return {
        builder_.CreateCall(llvm::Intrinsic::getOrInsertDeclaration(
                                builder_.GetInsertBlock()->getModule(),
                                llvm::Intrinsic::abs, {getValue()->getType()}),
                            {getValue(), builder_.getFalse()}),
        builder_};
  }

  explicit operator Float() const;
};
} // namespace MyDSL
