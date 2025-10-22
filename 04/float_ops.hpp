#pragma once

#include <cmath>
#include <ostream>

#include "base_ops.hpp"
#include "bool_ops.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

namespace MyDSL {
class Integer;

class Float : public BaseOps {
public:
  using NativeType = float;

private:
  Float getConst(NativeType value) const {
    return {llvm::ConstantFP::get(builder_.getContext(), llvm::APFloat(value)),
            builder_};
  }

  /// Ref constructor.
  Float(llvm::Type *type, llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(type, value, builder) {}

  template <class T> friend class Ref;

public:
  Float(NativeType value, llvm::IRBuilder<> &builder)
      : BaseOps(
            llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(value)),
            builder) {}
  Float(llvm::Value *value, llvm::IRBuilder<> &builder)
      : BaseOps(value, builder) {}

  explicit Float(const BaseOps &base) : BaseOps(base) {
    assert(base.getType()->isFloatTy());
  }

  /// Get the type, that all Floats have in our DSL.
  static llvm::Type *getType(llvm::LLVMContext &Ctx) {
    return llvm::Type::getFloatTy(Ctx);
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

  /// arithmetic operators
  // todo: implement me
  Float operator+(const Float &other) const {
    return {getValue(), builder_};
  }
  // todo: implement me
  Float operator-(const Float &other) const {
    return {getValue(), builder_};
  }
  // todo: implement me
  Float operator*(const Float &other) const {
    return {getValue(), builder_};
  }
  // todo: implement me
  Float operator/(const Float &other) const {
    return {getValue(), builder_};
  }

  Float operator+(NativeType f) const { return *this + getConst(f); }
  Float operator-(NativeType f) const { return *this - getConst(f); }
  Float operator*(NativeType f) const { return *this * getConst(f); }
  Float operator/(NativeType f) const { return *this / getConst(f); }

  /// compound assignment operators
  // todo: implement me
  Float &operator+=(const Float &other) {
    auto newValue = getValue();
    store(newValue);
    return *this;
  }
  // todo: implement me
  Float &operator-=(const Float &other) {
    auto newValue = getValue();
    store(newValue);
    return *this;
  }
  // todo: implement me
  Float &operator*=(const Float &other) {
    auto newValue = getValue();
    store(newValue);
    return *this;
  }
  // todo: implement me
  Float &operator/=(const Float &other) {
    auto newValue = getValue();
    store(newValue);
    return *this;
  }

  Float &operator+=(NativeType f) { return *this += getConst(f); }
  Float &operator-=(NativeType f) { return *this -= getConst(f); }
  Float &operator*=(NativeType f) { return *this *= getConst(f); }
  Float &operator/=(NativeType f) { return *this /= getConst(f); }

  Float operator^(const Float &other) const {
    return {
        builder_.CreateCall(llvm::Intrinsic::getOrInsertDeclaration(
                                builder_.GetInsertBlock()->getModule(),
                                llvm::Intrinsic::pow, {getValue()->getType()}),
                            {getValue(), other.getValue()}),
        builder_};
  }

  Float operator^(NativeType f) const { return *this ^ getConst(f); }

  // todo: implement me
  Float sqrt() const { return {getValue(), builder_}; }

  // todo: implement me
  Float abs() const { return {getValue(), builder_}; }

  explicit operator Integer() const;
};

} // namespace MyDSL
