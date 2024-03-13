#pragma once

#include "base_ops.hpp"
#include "bool_ops.hpp"

#include <concepts>

namespace MyDSL {

template <class T, class U>
concept Addable = requires(T t, U other) {
  { t + other } -> std::same_as<T>;
} || requires(T t, U other) {
  { t + other } -> std::same_as<U>;
};

template <class T, class U>
concept Subtractable = requires(T t, U other) {
  { t - other } -> std::same_as<T>;
} || requires(T t, U other) {
  { t - other } -> std::same_as<U>;
};

template <class T, class U>
concept Multiplicable = requires(T t, U other) {
  { t *other } -> std::same_as<T>;
} || requires(T t, U other) {
  { t *other } -> std::same_as<U>;
};

template <class T, class U>
concept Divisible = requires(T t, U other) {
  { t / other } -> std::same_as<T>;
} || requires(T t, U other) {
  { t / other } -> std::same_as<U>;
};
template <class T, class U>
concept Remaindable = requires(T t, U other) {
  { t % other } -> std::same_as<T>;
} || requires(T t, U other) {
  { t % other } -> std::same_as<U>;
};
template <class T, class U>
concept Powable = requires(T t, U other) {
  { t ^ other } -> std::same_as<T>;
} || requires(T t, U other) {
  { t ^ other } -> std::same_as<U>;
};

template <class T, class U>
concept EqualityComparable = requires(T t, U u) {
  { t == u } -> std::same_as<Bool>;
  { t != u } -> std::same_as<Bool>;
  { u == t } -> std::same_as<Bool>;
  { u != t } -> std::same_as<Bool>;
};

template <class T, class U>
concept PartiallyOrdered = requires(T t, U u) {
  { t < u } -> std::same_as<Bool>;
  { t > u } -> std::same_as<Bool>;
  { t <= u } -> std::same_as<Bool>;
  { t >= u } -> std::same_as<Bool>;
  { u < t } -> std::same_as<Bool>;
  { u > t } -> std::same_as<Bool>;
  { u <= t } -> std::same_as<Bool>;
  { u >= t } -> std::same_as<Bool>;
};

template <class T> class Ref {
  std::shared_ptr<T> ptr_;

public:
  Ref(llvm::Type *type, llvm::Value *value, llvm::IRBuilder<> &builder)
      : ptr_(new T(type, value, builder)) {}

  operator T &() { return *ptr_; }
  T &operator*() { return *ptr_; }

  Ref &operator=(const T &other) {
    *ptr_ = other;
    return *this;
  }

  // arithmetic operators
  T operator+(const T &other) const
    requires(Addable<T, T>)
  {
    return *ptr_ + other;
  }
  T operator-(const T &other) const
    requires(Subtractable<T, T>)
  {
    return *ptr_ - other;
  }
  T operator*(const T &other) const
    requires(Multiplicable<T, T>)
  {
    return *ptr_ * other;
  }
  T operator/(const T &other) const
    requires(Divisible<T, T>)
  {
    return *ptr_ / other;
  }
  T operator%(const T &other) const
    requires(Remaindable<T, T>)
  {
    return *ptr_ % other;
  }
  T operator^(const T &other) const
    requires(Powable<T, T>)
  {
    return *ptr_ ^ other;
  }

  T operator+(typename T::NativeType other) const
    requires(Addable<T, typename T::NativeType>)
  {
    return *ptr_ + other;
  }
  T operator-(typename T::NativeType other) const
    requires(Subtractable<T, typename T::NativeType>)
  {
    return *ptr_ - other;
  }
  T operator*(typename T::NativeType other) const
    requires(Multiplicable<T, typename T::NativeType>)
  {
    return *ptr_ * other;
  }
  T operator/(typename T::NativeType other) const
    requires(Divisible<T, typename T::NativeType>)
  {
    return *ptr_ / other;
  }
  T operator%(typename T::NativeType other) const
    requires(Remaindable<T, typename T::NativeType>)
  {
    return *ptr_ % other;
  }
  T operator^(typename T::NativeType other) const
    requires(Powable<T, typename T::NativeType>)
  {
    return *ptr_ ^ other;
  }

  // compound assignment operators
  T operator+=(const T &other)
    requires(Addable<T, T>)
  {
    return *ptr_ += other;
  }
  T operator-=(const T &other)
    requires(Subtractable<T, T>)
  {
    return *ptr_ -= other;
  }
  T operator*=(const T &other)
    requires(Multiplicable<T, T>)
  {
    return *ptr_ *= other;
  }
  T operator/=(const T &other)
    requires(Divisible<T, T>)
  {
    return *ptr_ /= other;
  }
  T operator%=(const T &other)
    requires(Remaindable<T, T>)
  {
    return *ptr_ %= other;
  }
  T operator^=(const T &other)
    requires(Powable<T, T>)
  {
    return *ptr_ ^= other;
  }

  T operator+=(typename T::NativeType other)
    requires(Addable<T, typename T::NativeType>)
  {
    return *ptr_ += other;
  }
  T operator-=(typename T::NativeType other)
    requires(Subtractable<T, typename T::NativeType>)
  {
    return *ptr_ -= other;
  }
  T operator*=(typename T::NativeType other)
    requires(Multiplicable<T, typename T::NativeType>)
  {
    return *ptr_ *= other;
  }
  T operator/=(typename T::NativeType other)
    requires(Divisible<T, typename T::NativeType>)
  {
    return *ptr_ /= other;
  }
  T operator%=(typename T::NativeType other)
    requires(Remaindable<T, typename T::NativeType>)
  {
    return *ptr_ %= other;
  }
  T operator^=(typename T::NativeType other)
    requires(Powable<T, typename T::NativeType>)
  {
    return *ptr_ ^= other;
  }

  // comparison operators
  Bool operator==(const T &other) const
    requires(EqualityComparable<T, T>)
  {
    return *ptr_ == other;
  }
  Bool operator!=(const T &other) const
    requires(EqualityComparable<T, T>)
  {
    return *ptr_ != other;
  }
  Bool operator<(const T &other) const
    requires(PartiallyOrdered<T, T>)
  {
    return *ptr_ < other;
  }
  Bool operator<=(const T &other) const
    requires(PartiallyOrdered<T, T>)
  {
    return *ptr_ <= other;
  }
  Bool operator>(const T &other) const
    requires(PartiallyOrdered<T, T>)
  {
    return *ptr_ > other;
  }
  Bool operator>=(const T &other) const
    requires(PartiallyOrdered<T, T>)
  {
    return *ptr_ >= other;
  }

  Bool operator==(typename T::NativeType other) const
    requires(EqualityComparable<T, typename T::NativeType>)
  {
    return *ptr_ == other;
  }
  Bool operator!=(typename T::NativeType other) const
    requires(EqualityComparable<T, typename T::NativeType>)
  {
    return *ptr_ != other;
  }
  Bool operator<(typename T::NativeType other) const
    requires(PartiallyOrdered<T, typename T::NativeType>)
  {
    return *ptr_ < other;
  }
  Bool operator<=(typename T::NativeType other) const
    requires(PartiallyOrdered<T, typename T::NativeType>)
  {
    return *ptr_ <= other;
  }
  Bool operator>(typename T::NativeType other) const
    requires(PartiallyOrdered<T, typename T::NativeType>)
  {
    return *ptr_ > other;
  }
  Bool operator>=(typename T::NativeType other) const
    requires(PartiallyOrdered<T, typename T::NativeType>)
  {
    return *ptr_ >= other;
  }
};
} // namespace MyDSL
