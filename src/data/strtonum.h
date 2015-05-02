/*!
 *  Copyright (c) 2015 by Contributors
 * \file strtonum.h
 * \brief A faster implementation
 */
#ifndef DMLC_DATA_STRTONUM_H_
#define DMLC_DATA_STRTONUM_H_

namespace dmlc {
namespace data {

inline bool isspace(char c) {
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f');
}

inline bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

inline double atof (const char *p) {
  // Skip leading white space, if any. Not necessary
  // while (isspace(*p) ) ++ p;

  // Get sign, if any.
  double sign = 1.0;
  if (*p == '-') {
    sign = -1.0; ++ p;
  } else if (*p == '+') {
    ++ p;
  }

  // Get digits before decimal point or exponent, if any.
  unsigned long long val1;
  for (val1 = 0; isdigit(*p); ++p) {
    val1 = val1 * 10 + (*p - '0');
  }
  double value = (double)val1;

  // Get digits after decimal point, if any.
  if (*p == '.') {
    unsigned long long pow10 = 10;
    unsigned long long val2 = 0;
    ++ p;
    while (isdigit(*p)) {
      val2 = val2 * 10 + (*p - '0');
      pow10 *= 10;
      ++ p;
    }
    value += (double)val2 / (double)pow10;
  }

  // Handle exponent, if any.
  if ((*p == 'e') || (*p == 'E')) {
    int frac = 0;
    double scale = 1.0;
    unsigned int expon;
    // Get sign of exponent, if any.
    p += 1;
    if (*p == '-') {
      frac = 1;
      p += 1;
    } else if (*p == '+') {
      p += 1;
    }
    // Get digits of exponent, if any.
    for (expon = 0; isdigit(*p); p += 1) {
      expon = expon * 10 + (*p - '0');
    }
    if (expon > 308) expon = 308;
    // Calculate scaling factor.
    while (expon >= 50) { scale *= 1E50; expon -= 50; }
    while (expon >=  8) { scale *= 1E8;  expon -=  8; }
    while (expon >   0) { scale *= 10.0; expon -=  1; }
    // Return signed and scaled floating point result.
    return sign * (frac ? (value / scale) : (value * scale));
  } else {
    return sign > 0 ? value : - value;
  }
}

inline long atol(const char* p) {
  // Get sign if any
  bool sign = true;
  if (*p == '-') {
    sign = false; ++ p;
  } else if (*p == '+') {
    ++ p;
  }

  long value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * 10 + (*p - '0');
  }
  return sign ? value : - value;
}


}  // namespace data
}  // namespace dmlc

#endif /* DMLC_DATA_STRTONUM_H_ */
