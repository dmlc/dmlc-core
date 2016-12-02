/*!
 * Copyright (c) 2016 by Contributors
 * \file optional.h
 * \brief Container to hold optional data.
 */
#ifndef DMLC_OPTIONAL_H_
#define DMLC_OPTIONAL_H_

#include <iostream>
#include <utility>
#include <algorithm>

#include "./base.h"
#include "./logging.h"
#include "./type_traits.h"

namespace dmlc {

struct nullopt_t {
  constexpr nullopt_t(int) {}
};

constexpr nullopt_t nullopt(0);

template <typename T>
class optional {
 public:
  optional() : val(nullptr) {}
  explicit optional(const T& value) : val(new T(value)) {}
  optional (const optional<T>& other) {
    if (other) {
      val = new T(*other);
    } else {
      val = nullptr;
    }
  }
  ~optional() {
    if (val != nullptr) delete val;
  }
  optional<T>& operator=(const T& value) {
    if (val != nullptr) {
      delete val;
    }
    val = new T(value);
    return *this;
  }
  optional<T>& operator=(const optional<T> &other) {
    if (other.val == nullptr) {
      val = nullptr;
    } else {
      *this = *other;
    }
    return *this;
  }
  optional<T>& operator=(nullopt_t) {
    if (val != nullptr) {
      delete val;
      val = nullptr;
    }
    return *this;
  }
  constexpr T& operator*() const { return *val; }
  const T& value() const {
    if (val == nullptr) {
      throw std::logic_error("bad optional access");
    }
    return *val;
  }
  explicit operator bool() const { return val != nullptr; }
  friend std::ostream &operator<<(std::ostream &os, const optional<T> &t) {
    if (bool(t)) {
      os << *t;
    } else {
      os << "None";
    }
    return os;
  }
  friend std::istream &operator>>(std::istream &is, optional<T> &t) {
    char buf[4];
    std::streampos origin = is.tellg();
    is.read(buf, 4);
    if (is.fail() || buf[0] != 'N' || buf[1] != 'o' ||
        buf[2] != 'n' || buf[3] != 'e') {
      is.clear();
      is.seekg(origin);
      T x;
      is >> x;
      t = x;
      return is;
    } else {
      t = nullopt;
    }
    return is;
  }
 private:
  T *val;
};

DMLC_DECLARE_TYPE_NAME(optional<int>, "optional<int>");

}  // namespace optional

#endif  // DMLC_OPTIONAL_H_
