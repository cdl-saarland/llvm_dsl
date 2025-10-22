#pragma once
#include <cmath>
#include <cstdint>
#include <ostream>

#include "bool_ops.hpp"

namespace MyDSL {

class Integer {
  std::int64_t value_;

public:
  Integer(std::int64_t value) : value_(value) {}

  Integer operator+(const Integer &other) const {
    return Integer(value_ + other.value_);
  }
  Integer operator-(const Integer &other) const {
    return Integer(value_ - other.value_);
  }
  Integer operator*(const Integer &other) const {
    return Integer(value_ * other.value_);
  }

  Integer operator-() const { return Integer(-value_); }

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

  // the missing operators
  Integer operator%(std::int64_t other) const { return value_ % other; }

  Integer &operator/=(std::int64_t other) {
    value_ /= other;
    return *this;
  }
  // end of the missing operators

  bool operator==(const Integer &other) const { return value_ == other.value_; }
  bool operator!=(const Integer &other) const { return value_ != other.value_; }
  bool operator<(const Integer &other) const { return value_ < other.value_; }
  bool operator<=(const Integer &other) const { return value_ <= other.value_; }
  bool operator>(const Integer &other) const { return value_ > other.value_; }
  bool operator>=(const Integer &other) const { return value_ >= other.value_; }

  std::int64_t getValue() const { return value_; }
  explicit operator std::int64_t() const { return getValue(); }

  friend std::ostream &operator<<(std::ostream &os, const Integer &f) {
    os << f.value_;
    return os;
  }
};

} // namespace MyDSL
