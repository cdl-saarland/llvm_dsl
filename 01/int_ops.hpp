#pragma once
#include <cmath>
#include <cstdint>
#include <ostream>

#include "bool_ops.hpp"

namespace MyDSL {

class Float;

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

  // todo: implement missing operators

  std::int64_t getValue() const { return value_; }
  operator std::int64_t() const { return getValue(); }

  friend std::ostream &operator<<(std::ostream &os, const Integer &f) {
    os << f.value_;
    return os;
  }
};

} // namespace MyDSL
