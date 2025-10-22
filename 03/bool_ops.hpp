#pragma once

#include <cmath>
#include <ostream>

namespace MyDSL {

class Bool {
  bool value_;

public:
  Bool(bool value) : value_(value) {}

  Bool operator&&(const Bool &other) const {
    return Bool(value_ && other.value_);
  }
  Bool operator||(const Bool &other) const {
    return Bool(value_ || other.value_);
  }

  Bool operator!() const { return Bool(!value_); }
  Bool operator==(const Bool &other) const { return value_ == other.value_; }
  Bool operator!=(const Bool &other) const { return value_ != other.value_; }
  Bool operator<(const Bool &other) const { return value_ < other.value_; }
  Bool operator<=(const Bool &other) const { return value_ <= other.value_; }
  Bool operator>(const Bool &other) const { return value_ > other.value_; }
  Bool operator>=(const Bool &other) const { return value_ >= other.value_; }

  Bool &operator|=(const Bool &other) {
    value_ |= other.value_;
    return *this;
  }
  Bool &operator&=(const Bool &other) {
    value_ &= other.value_;
    return *this;
  }
  Bool &operator^=(const Bool &other) {
    value_ ^= other.value_;
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const Bool &b) {
    os << b.value_;
    return os;
  }
  operator bool() const { return value_; }
  operator int() const { return value_; }
};
} // namespace MyDSL
