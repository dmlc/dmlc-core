/*!
 * Copyright (c) 2015-2018 by Contributors
 * \file strtonum.h
 * \brief A faster implementation of strtof and strtod
 */
#ifndef DMLC_STRTONUM_H_
#define DMLC_STRTONUM_H_

#if DMLC_USE_CXX11
#include <type_traits>
#endif

#include <limits>
#include <cstdint>
#include "./base.h"
#include "./logging.h"

namespace dmlc {
inline bool isspace(char c) {
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f');
}

inline bool isblank(char c) {
  return (c == ' ' || c == '\t');
}

inline bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

inline bool isdigitchars(char c) {
  return (c >= '0' && c <= '9')
    || c == '+' || c == '-'
    || c == '.'
    || c == 'e' || c == 'E';
}

const int kStrtofMaxDigits = 19;

/*!
 * \brief A faster version of strtof / strtod
 * TODO the current version does not support INF, NAN, and hex number
 */
template <typename FloatType, bool CheckRange = false>
inline FloatType ParseFloat(const char* nptr, char** endptr) {
#if DMLC_USE_CXX11
  static_assert(std::is_same<FloatType, double>::value
                || std::is_same<FloatType, float>::value,
               "ParseFloat is defined only for 'float' and 'double' types");
  constexpr int kMaxExponent
    = (std::is_same<FloatType, double>::value ? 308 : 38);
#else
  const int kMaxExponent = (sizeof(FloatType) == sizeof(double) ? 308 : 38);
#endif

  const char *p = nptr;
  // Skip leading white space, if any. Not necessary
  while (isspace(*p) ) ++p;

  // Get sign, if any.
  bool sign = true;
  if (*p == '-') {
    sign = false; ++p;
  } else if (*p == '+') {
    ++p;
  }

  // Get digits before decimal point or exponent, if any.
  FloatType value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * 10.0f + (*p - '0');
  }

  // Get digits after decimal point, if any.
  if (*p == '.') {
    uint64_t pow10 = 1;
    uint64_t val2 = 0;
    int digit_cnt = 0;
    ++p;
    while (isdigit(*p)) {
      if (digit_cnt < kStrtofMaxDigits) {
        val2 = val2 * 10 + (*p - '0');
        pow10 *= 10;
      }  // when kStrtofMaxDigits is read, ignored following digits
      ++p;
      ++digit_cnt;
    }
    value += static_cast<FloatType>(
        static_cast<double>(val2) / static_cast<double>(pow10));
  }

  // Handle exponent, if any.
  if ((*p == 'e') || (*p == 'E')) {
    ++p;
    bool frac = false;
    FloatType scale = 1.0;
    unsigned expon;
    // Get sign of exponent, if any.
    if (*p == '-') {
      frac = true;
      ++p;
    } else if (*p == '+') {
      ++p;
    }
    // Get digits of exponent, if any.
    for (expon = 0; isdigit(*p); p += 1) {
      expon = expon * 10 + (*p - '0');
    }
    if (expon > kMaxExponent) {  // out of range, clip or raise error
      if (CheckRange) {
        errno = ERANGE;
        if (endptr) *endptr = (char*)p;  // NOLINT(*)
        return std::numeric_limits<FloatType>::infinity();
      }
      expon = kMaxExponent;
    }
    // Calculate scaling factor.
    while (expon >=  8) { scale *= 1E8;  expon -=  8; }
    while (expon >   0) { scale *= 10.0; expon -=  1; }
    // Return signed and scaled floating point result.
    value = frac ? (value / scale) : (value * scale);
  }

  if (endptr) *endptr = (char*)p;  // NOLINT(*)
  return sign ? value : - value;
}

/*!
 * \brief A faster version of strtof
 */
inline float strtof(const char* nptr, char** endptr) {
  return ParseFloat<float>(nptr, endptr);
}

/*!
 * \brief A faster version of strtof, with range checking
 */
inline float strtof_check_range(const char* nptr, char** endptr) {
  return ParseFloat<float, true>(nptr, endptr);
}

/*!1
 * \brief A faster version of strtod
 */
inline double strtod(const char* nptr, char** endptr) {
  return ParseFloat<double>(nptr, endptr);
}

/*!
 * \brief A faster version of strtod, with range checking
 */
inline double strtod_check_range(const char* nptr, char** endptr) {
  return ParseFloat<double, true>(nptr, endptr);
}

/**
 * \brief A faster string to integer convertor
 * TODO only support base <=10
 */
template <typename V>
inline V strtoint(const char* nptr, char **endptr, int base) {
  const char *p = nptr;
  // Skip leading white space, if any. Not necessary
  while (isspace(*p) ) ++p;

  // Get sign if any
  bool sign = true;
  if (*p == '-') {
    sign = false; ++p;
  } else if (*p == '+') {
    ++p;
  }

  V value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * base + (*p - '0');
  }

  if (endptr) *endptr = (char*)p; // NOLINT(*)
  return sign ? value : - value;
}

template <typename V>
inline V strtouint(const char* nptr, char **endptr, int base) {
  const char *p = nptr;
  // Skip leading white space, if any. Not necessary
  while (isspace(*p)) ++p;

  // Get sign if any
  bool sign = true;
  if (*p == '-') {
    sign = false; ++p;
  } else if (*p == '+') {
    ++p;
  }

  // we are parsing unsigned, so no minus sign should be found
  CHECK_EQ(sign, true);

  V value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * base + (*p - '0');
  }

  if (endptr) *endptr = (char*)p; // NOLINT(*)
  return value;
}

inline uint64_t
strtoull(const char* nptr, char **endptr, int base) {
  return strtouint<uint64_t>(nptr, endptr, base);
}

inline long atol(const char* p) {  // NOLINT(*)
  return strtoint<long>(p, 0, 10); // NOLINT(*)
}

inline float atof(const char *nptr) {
  return strtof(nptr, 0);
}


template<typename T>
class Str2T {
 public:
  static inline T get(const char * begin, const char * end);
};

template<typename T>
inline T Str2Type(const char * begin, const char * end) {
  return Str2T<T>::get(begin, end);
}

template<>
class Str2T<int32_t> {
 public:
  static inline int32_t get(const char * begin, const char * end) {
    return strtoint<int>(begin, NULL, 10);
  }
};

template<>
class Str2T<uint32_t> {
 public:
  static inline uint32_t get(const char * begin, const char * end) {
    return strtouint<int>(begin, NULL, 10);
  }
};

template<>
class Str2T<int64_t> {
 public:
  static inline int64_t get(const char * begin, const char * end) {
    return strtoint<int64_t>(begin, NULL, 10);
  }
};

template<>
class Str2T<uint64_t> {
 public:
  static inline uint64_t get(const char * begin, const char * end) {
    return strtouint<uint64_t>(begin, NULL, 10);
  }
};


template<>
class Str2T<float> {
 public:
  static inline float get(const char * begin, const char * end) {
    return atof(begin);
  }
};

/**
* \brief Parse colon seperated pair v1[:v2]
* \param begin: pointer to string
* \param end: one past end of string
* \param parseEnd: end string of parsed string
* \param v1: first value in the pair
* \param v2: second value in the pair
* \output number of values parsed
*/
template<typename T1, typename T2>
inline int ParsePair(const char * begin, const char * end,
                     const char ** endptr, T1 &v1, T2 &v2) { // NOLINT(*)
  const char * p = begin;
  while (p != end && !isdigitchars(*p)) ++p;
  if (p == end) {
    *endptr = end;
    return 0;
  }
  const char * q = p;
  while (q != end && isdigitchars(*q)) ++q;
  v1 = Str2Type<T1>(p, q);
  p = q;
  while (p != end && isblank(*p)) ++p;
  if (p == end || *p != ':') {
    // only v1
    *endptr = p;
    return 1;
  }
  p++;
  while (p != end && !isdigitchars(*p)) ++p;
  q = p;
  while (q != end && isdigitchars(*q)) ++q;
  *endptr = q;
  v2 = Str2Type<T2>(p, q);
  return 2;
}

/**
* \brief Parse colon seperated triple v1:v2[:v3]
* \param begin: pointer to string
* \param end: one past end of string
* \param parseEnd: end string of parsed string
* \param v1: first value in the triple
* \param v2: second value in the triple
* \param v3: third value in the triple
* \output number of values parsed
*/
template<typename T1, typename T2, typename T3>
inline int ParseTriple(const char * begin, const char * end,
                     const char ** endptr, T1 &v1, T2 &v2, T3 &v3) { // NOLINT(*)
  const char * p = begin;
  while (p != end && !isdigitchars(*p)) ++p;
  if (p == end) {
    *endptr = end;
    return 0;
  }
  const char * q = p;
  while (q != end && isdigitchars(*q)) ++q;
  v1 = Str2Type<T1>(p, q);
  p = q;
  while (p != end && isblank(*p)) ++p;
  if (p == end || *p != ':') {
    // only v1
    *endptr = p;
    return 1;
  }
  p++;
  while (p != end && !isdigitchars(*p)) ++p;
  q = p;
  while (q != end && isdigitchars(*q)) ++q;
  v2 = Str2Type<T2>(p, q);
  p = q;
  while (p != end && isblank(*p)) ++p;
  if (p == end || *p != ':') {
    // only v1:v2
    *endptr = p;
    return 2;
  }
  p++;
  while (p != end && !isdigitchars(*p)) ++p;
  q = p;
  while (q != end && isdigitchars(*q)) ++q;
  *endptr = q;
  v3 = Str2Type<T3>(p, q);
  return 3;
}
}  // namespace dmlc

#endif  // DMLC_STRTONUM_H_
