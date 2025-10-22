#pragma once

#include "base_ops.hpp"
#include "control_flow.hpp"
#include "int_ops.hpp"
#include "ref.hpp"

#include <algorithm>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>

namespace MyDSL {

class Float;

template <class T, int Dim> class Tensor {
  llvm::SmallVector<Integer, Dim> size_;
  llvm::Value *data_;

  llvm::IRBuilder<> &builder_;

  template <class U, int D> friend class Tensor;

public:
  using NativeType = typename T::NativeType *;

  Tensor(const llvm::SmallVector<Integer, Dim> &size, llvm::Value *data,
         llvm::IRBuilder<> &builder)
      : size_(size), data_(data), builder_(builder) {}

  Tensor(const llvm::SmallVector<Integer, Dim> &size,
         llvm::IRBuilder<> &builder)
      : size_(size), builder_(builder) {

    // auto &EntryBB = builder.GetInsertBlock()->getParent()->getEntryBlock();

    Integer totalSize = size[0];
    for (const auto &i : llvm::drop_begin(size))
      totalSize *= i;

    data_ = builder_.CreateAlloca(getScalarType(builder_.getContext()),
                                  totalSize, "tensor_data");
  }

  static llvm::Type *getScalarType(llvm::LLVMContext &Ctx) {
    return T::getType(Ctx);
  }

  static llvm::Type *getType(llvm::LLVMContext &Ctx) {
    return llvm::PointerType::getUnqual(getScalarType(Ctx));
  }

  Tensor<T, Dim - 1> operator[](Integer index)
    requires(Dim > 1)
  {
    for (const auto &i : llvm::drop_begin(size_))
      index *= i;
    auto GEP = builder_.CreateGEP(getScalarType(builder_.getContext()), data_,
                                  {index}, "tensor_index." + llvm::Twine{Dim},
                                  /*inbounds=*/true);

    llvm::SmallVector<Integer, Dim - 1> newSize(size_.begin() + 1, size_.end());

    return Tensor<T, Dim - 1>(newSize, GEP, builder_);
  }

  Tensor<T, Dim - 1> operator[](int index)
    requires(Dim > 1)
  {
    return operator[](Integer(index, builder_));
  }

  Ref<T> operator[](const Integer &index)
    requires(Dim == 1)
  {
    auto GEP = builder_.CreateGEP(getScalarType(builder_.getContext()), data_,
                                  {index}, "tensor_index." + llvm::Twine{Dim},
                                  /*inbounds=*/true);
    return {getScalarType(builder_.getContext()), GEP, builder_};
  }

  Ref<T> operator[](int index)
    requires(Dim == 1)
  {
    return operator[](Integer(index, builder_));
  }

  // This is not const-correct...
  Tensor<T, Dim - 1> operator[](const Integer &index) const
    requires(Dim > 1)
  {
    return const_cast<Tensor<T, Dim> *>(this)->operator[](index);
  }

  Tensor<T, Dim - 1> operator[](int index) const
    requires(Dim > 1)
  {
    return const_cast<Tensor<T, Dim> *>(this)->operator[](index);
  }

  // This is const-correct, as we take a copy, not a Ref...
  T operator[](const Integer &index) const
    requires(Dim == 1)
  {
    return const_cast<Tensor<T, Dim> *>(this)->operator[](index);
  }

  T operator[](int index) const
    requires(Dim == 1)
  {
    return const_cast<Tensor<T, Dim> *>(this)->operator[](index);
  }

  operator llvm::Value *() const { return data_; }

private:
  template <class F>
  void elementwiseOp(Tensor<T, Dim> &dest, const Tensor<T, Dim> &other,
                     F &&f) const {
    ControlFlow CF(builder_);

    if constexpr (Dim == 1) {
      CF.For(
          Integer{0, builder_}, [&](Integer i) { return i < size_[0]; },
          [&](Integer i) { return i + 1; },
          [&](Integer i) {
            dest[i] = std::forward<F>(f)((*this)[i], other[i]);
          });
    } else {
      CF.For(
          Integer{0, builder_}, [&](Integer i) { return i < size_[0]; },
          [&](Integer i) { return i + 1; },
          [&](Integer i) {
            Tensor<T, Dim - 1> dst(dest[i]);
            (*this)[i].elementwiseOp(dst, other[i], std::forward<F>(f));
          });
    }
  }

  template <class F>
  void elementwiseOp(Tensor<T, Dim> &dest, const T &other, F &&f) const {
    ControlFlow CF(builder_);

    if constexpr (Dim == 1) {
      CF.For(
          Integer{0, builder_}, [&](Integer i) { return i < size_[0]; },
          [&](Integer i) { return i + 1; },
          [&](Integer i) { dest[i] = std::forward<F>(f)((*this)[i], other); });
    } else {
      CF.For(
          Integer{0, builder_}, [&](Integer i) { return i < size_[0]; },
          [&](Integer i) { return i + 1; },
          [&](Integer i) {
            Tensor<T, Dim - 1> dst(dest[i]);
            (*this)[i].elementwiseOp(dst, other, std::forward<F>(f));
          });
    }
  }

public:
  Tensor<T, Dim> operator*(const Tensor<T, Dim> &other) const
    requires(Multiplicable<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a * b; });
    return result;
  }

  Tensor<T, Dim> operator*(const T &other) const
    requires(Multiplicable<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a * b; });
    return result;
  }

  Tensor<T, Dim> operator/(const Tensor<T, Dim> &other) const
    requires(Divisible<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a / b; });
    return result;
  }

  Tensor<T, Dim> operator/(const T &other) const
    requires(Divisible<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a / b; });
    return result;
  }

  Tensor<T, Dim> operator+(const Tensor<T, Dim> &other) const
    requires(Addable<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a + b; });
    return result;
  }

  Tensor<T, Dim> operator+(const T &other) const
    requires(Addable<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a + b; });
    return result;
  }

  Tensor<T, Dim> operator-(const Tensor<T, Dim> &other) const
    requires(Subtractable<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a - b; });
    return result;
  }

  Tensor<T, Dim> operator-(const T &other) const
    requires(Subtractable<T, T>)
  {
    Tensor<T, Dim> result(size_, builder_);
    elementwiseOp(result, other,
                  [](const auto &a, const auto &b) { return a - b; });
    return result;
  }

  Tensor<T, Dim> &operator*=(const Tensor<T, Dim> &other)
    requires(Multiplicable<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a * b; });
    return *this;
  }

  Tensor<T, Dim> &operator*=(const T &other)
    requires(Multiplicable<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a * b; });
    return *this;
  }

  Tensor<T, Dim> &operator/=(const Tensor<T, Dim> &other)
    requires(Divisible<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a / b; });
    return *this;
  }

  Tensor<T, Dim> &operator/=(const T &other)
    requires(Divisible<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a / b; });
    return *this;
  }

  Tensor<T, Dim> &operator+=(const Tensor<T, Dim> &other)
    requires(Addable<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a + b; });
    return *this;
  }

  Tensor<T, Dim> &operator+=(const T &other)
    requires(Addable<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a + b; });
    return *this;
  }

  Tensor<T, Dim> &operator-=(const Tensor<T, Dim> &other)
    requires(Subtractable<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a - b; });
    return *this;
  }

  Tensor<T, Dim> &operator-=(const T &other)
    requires(Subtractable<T, T>)
  {
    elementwiseOp(*this, other,
                  [](const auto &a, const auto &b) { return a - b; });
    return *this;
  }

  void mmul(Tensor<T,Dim>& dest, const Tensor<T, Dim> &other) const
    requires(Multiplicable<T, T> && Addable<T, T> && Dim == 2)
  {
    // pre: size_[0] and size_[1] are equiv

    auto &M = *builder_.GetInsertBlock()->getModule();
    auto &Ctx = M.getContext();

    // todo: implement call to __mydsl_tensor_mmul_2_f32
  }
};
} // namespace MyDSL
