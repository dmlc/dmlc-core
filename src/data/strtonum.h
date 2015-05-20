/*!
 *  Copyright (c) 2015 by Contributors
 * \file strtonum.h
 * \brief A faster implementation of strtod, ...
 */
#ifndef DMLC_DATA_STRTONUM_H_
#define DMLC_DATA_STRTONUM_H_
#include "dmlc/base.h"
namespace dmlc {
namespace data {

inline bool isspace(char c) {
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f');
}

inline bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

/*!
 * \brief A faster version of strtof
 * TODO the current version does not support INF, NAN, and hex number
 */
inline float strtof(const char *nptr, char **endptr) {
  const char *p = nptr;
  // Skip leading white space, if any. Not necessary
  while (isspace(*p) ) ++ p;

  // Get sign, if any.
  bool sign = true;
  if (*p == '-') {
    sign = false; ++ p;
  } else if (*p == '+') {
    ++ p;
  }

  // Get digits before decimal point or exponent, if any.
  float value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * 10.0 + (*p - '0');
  }

  // Get digits after decimal point, if any.
  if (*p == '.') {
    unsigned pow10 = 1;
    unsigned val2 = 0;
    ++ p;
    while (isdigit(*p)) {
      val2 = val2 * 10 + (*p - '0');
      pow10 *= 10;
      ++ p;
    }
    // std::cout << val2 << "  " << pow10 << std::endl;
    value += (float)val2 / (float)pow10;
  }

  // Handle exponent, if any.
  if ((*p == 'e') || (*p == 'E')) {
    ++ p;
    bool frac = false;
    float scale = 1.0;
    unsigned expon;
    // Get sign of exponent, if any.
    if (*p == '-') {
      frac = true;
      ++ p;
    } else if (*p == '+') {
      ++ p;
    }
    // Get digits of exponent, if any.
    for (expon = 0; isdigit(*p); p += 1) {
      expon = expon * 10 + (*p - '0');
    }
    if (expon > 38) expon = 38;
    // Calculate scaling factor.
    while (expon >=  8) { scale *= 1E8;  expon -=  8; }
    while (expon >   0) { scale *= 10.0; expon -=  1; }
    // Return signed and scaled floating point result.
    value = frac ? (value / scale) : (value * scale);
  }

  if (endptr) *endptr = (char*) p;
  return sign ? value : - value;
}

/**
 * \brief A faster string to integer convertor
 * TODO only support base <=10
 */
template <typename V>
inline V strtoint(const char* nptr, char **endptr, int base) {
  const char *p = nptr;
  // Skip leading white space, if any. Not necessary
  while (isspace(*p) ) ++ p;

  // Get sign if any
  bool sign = true;
  if (*p == '-') {
    sign = false; ++ p;
  } else if (*p == '+') {
    ++ p;
  }

  V value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * base + (*p - '0');
  }

  if (endptr) *endptr = (char*) p;
  return sign ? value : - value;
}

inline uint64_t
strtoull(const char* nptr, char **endptr, int base) {
  return strtoint<uint64_t>(nptr, endptr, base);
};

inline long atol(const char* p) {
  return strtoint<long>(p, 0, 10);
}

inline float atof(const char *nptr) {
  return strtof(nptr, 0);
}


}  // namespace data
}  // namespace dmlc

#endif /* DMLC_DATA_STRTONUM_H_ */
