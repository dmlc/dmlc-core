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
/*!
 * \brief Inline implementation of isspace(). Tests whether the given character
 *        is a whitespace letter.
 * \param c Character to test
 * \return Result of the test
 */
inline bool isspace(char c) {
  return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f');
}

/*!
 * \brief Inline implementation of isblank(). Tests whether the given character
 *        is a space or tab character.
 * \param c Character to test
 * \return Result of the test
 */
inline bool isblank(char c) {
  return (c == ' ' || c == '\t');
}

/*!
 * \brief Inline implementation of isdigit(). Tests whether the given character
 *        is a decimal digit
 * \param c Character to test
 * \return Result of the test
 */
inline bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

/*!
 * \brief Tests whether the given character is a valid letter in the string
 *        representation of a floating-point value, i.e. decimal digits,
 *        signs (+/-), decimal point (.), or exponent marker (e/E).
 * \param c Character to test
 * \return Result of the test
 */
inline bool isdigitchars(char c) {
  return (c >= '0' && c <= '9')
    || c == '+' || c == '-'
    || c == '.'
    || c == 'e' || c == 'E';
}

/*!
 * \brief Maximum number of decimal digits dmlc::strtof() / dmlc::strtod()
 *        will process. Trailing digits will be ignored.
 */
const int kStrtofMaxDigits = 19;

/*!
 * \brief Common implementation for dmlc::strtof() and dmlc::strtod()
 * TODO: the current version does not support INF, NAN, and hex number
 * \param nptr Beginning of the string that's to be converted into a
 *             floating-point number
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \return Converted floating-point value, in FloatType
 * \tparam FloatType Type of floating-point number to be obtained. This must
 *                   be either float or double.
 * \tparam CheckRange Whether to check for overflow. If set to true, an out-
 *                    of-range value will cause errno to be set to ERANGE and
 *                    ParseFloat() to return HUGE_VAL / HUGE_VALF; otherwise,
 *                    all out-of-range vlaues will be silently clipped.
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
 * \brief A faster implementation of strtof(). See documentation of
 *        std::strtof() for more information. Note that this function does not
 *        check for overflow. Use strtof_check_range() to check for overflow.
 * TODO: the current version does not support INF, NAN, and hex number
 * \param nptr Beginning of the string that's to be converted into float
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \return Converted floating-point value, in float type
 */
inline float strtof(const char* nptr, char** endptr) {
  return ParseFloat<float>(nptr, endptr);
}

/*!
 * \brief A faster implementation of strtof(). See documentation of
 *        std::strtof() for more information. This function will check for
 *        overflow. If the converted value is outside the range for the float
 *        type, errno is set to ERANGE and HUGE_VALF is returned.
 * TODO: the current version does not support INF, NAN, and hex number
 * \param nptr Beginning of the string that's to be converted into float
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \return Converted floating-point value, in float type
 */
inline float strtof_check_range(const char* nptr, char** endptr) {
  return ParseFloat<float, true>(nptr, endptr);
}

/*!
 * \brief A faster implementation of strtod(). See documentation of
 *        std::strtof() for more information. Note that this function does not
 *        check for overflow. Use strtod_check_range() to check for overflow.
 * TODO: the current version does not support INF, NAN, and hex number
 * \param nptr Beginning of the string that's to be converted into double
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \return Converted floating-point value, in double type
 */
inline double strtod(const char* nptr, char** endptr) {
  return ParseFloat<double>(nptr, endptr);
}

/*!
 * \brief A faster implementation of strtod(). See documentation of
 *        std::strtod() for more information. This function will check for
 *        overflow. If the converted value is outside the range for the double
 *        type, errno is set to ERANGE and HUGE_VAL is returned.
 * TODO: the current version does not support INF, NAN, and hex number
 * \param nptr Beginning of the string that's to be converted into double
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \return Converted floating-point value, in float type
 */
inline double strtod_check_range(const char* nptr, char** endptr) {
  return ParseFloat<double, true>(nptr, endptr);
}

/*!
 * \brief A fast string-to-integer convertor, for signed integers
 * TODO: the current version supports only base <= 10
 * \param nptr Beginning of the string that's to be converted into a signed
 *             integer
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \param base Base to use for integer conversion
 * \return Converted value, in SignedIntType
 * \tparam SignedIntType Type of signed integer to be obtained.
 */
template <typename SignedIntType>
inline SignedIntType ParseSignedInt(const char* nptr, char** endptr, int base) {
#ifdef DMLC_USE_CXX11
  static_assert(std::is_signed<SignedIntType>::value
                && std::is_integral<SignedIntType>::value,
                "ParseSignedInt is defined for signed integers only");
#endif
  CHECK_LE(base, 10);
  const char* p = nptr;
  // Skip leading white space, if any. Not necessary
  while (isspace(*p) ) ++p;

  // Get sign if any
  bool sign = true;
  if (*p == '-') {
    sign = false; ++p;
  } else if (*p == '+') {
    ++p;
  }

  SignedIntType value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * base + (*p - '0');
  }

  if (endptr) *endptr = (char*)p;  // NOLINT(*)
  return sign ? value : - value;
}

/*!
 * \brief A fast string-to-integer convertor, for unsigned integers
 * TODO: the current version supports only base <= 10
 * \param nptr Beginning of the string that's to be converted into an unsigned
 *             integer
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \param base Base to use for integer conversion
 * \return Converted value, in UnsignedIntType
 * \tparam UnsignedIntType Type of unsigned integer to be obtained.
 */
template <typename UnsignedIntType>
inline UnsignedIntType ParseUnsignedInt(const char* nptr, char** endptr, int base) {
#ifdef DMLC_USE_CXX11
  static_assert(std::is_unsigned<UnsignedIntType>::value
                && std::is_integral<UnsignedIntType>::value,
                "ParseUnsignedInt is defined for unsigned integers only");
#endif
  CHECK_LE(base, 10);
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

  UnsignedIntType value;
  for (value = 0; isdigit(*p); ++p) {
    value = value * base + (*p - '0');
  }

  if (endptr) *endptr = (char*)p; // NOLINT(*)
  return value;
}

/*!
 * \brief A faster implementation of strtoull(). See documentation of
 *        std::strtoull() for more information.
 * TODO: the current version supports only base <= 10
 * \param nptr Beginning of the string that's to be converted into integer of
 *             type unsigned long long
 * \param endptr After the conversion, this pointer will be set to point one
 *               past the last character used in the conversion.
 * \param base Base to use for integer conversion
 * \return Converted value, as unsigned 64-bit integer
 */
inline uint64_t strtoull(const char* nptr, char **endptr, int base) {
  return ParseUnsignedInt<uint64_t>(nptr, endptr, base);
}

/*!
 * \brief A faster implementation of atol(). See documentation of std::atol()
 *        for more information. This function will use base 10.
 * \param p Beginning of the string that's to be converted into integer of
 *          type long
 * \return Converted value, as long integer (width is system-dependent)
 */
inline long atol(const char* p) {  // NOLINT(*)
  return ParseSignedInt<long>(p, 0, 10); // NOLINT(*)
}

/*!
 * \brief A faster implementation of atof(). Unlike std::atof(), this function
 *        returns float type.
 * \param nptr Beginning of the string that's to be converted into float
 * \return Converted value, in float type
 */
inline float atof(const char* nptr) {
  return strtof(nptr, 0);
}

/*!
 * \brief Interface class that defines a single method get() to convert
 *        a string into type T. Define template specialization of this class
 *        to define the conversion method for a particular type.
 * \tparam Type of converted value
 */
template<typename T>
class Str2T {
 public:
  /*!
   * \brief Convert a string into type T
   * \param begin Beginning of the string to convert
   * \param end End of the string to convert
   * \return Converted value, in type T
   */
  static inline T get(const char * begin, const char * end);
};

/*!
 * \brief Convenience function for converting string into type T
 * \param begin Beginning of the string to convert
 * \param end End of the string to convert
 * \return Converted value, in type T
 * \tparam Type of converted value
 */
template<typename T>
inline T Str2Type(const char * begin, const char * end) {
  return Str2T<T>::get(begin, end);
}

/*!
 * \brief Template specialization of Str2T<> interface for signed 32-bit integer
 */
template<>
class Str2T<int32_t> {
 public:
  /*!
   * \brief Convert a string into signed 32-bit integer
   * \param begin Beginning of the string to convert
   * \param end End of the string to convert
   * \return Converted value, as signed 32-bit integer
   */
  static inline int32_t get(const char * begin, const char * end) {
    return ParseSignedInt<int32_t>(begin, NULL, 10);
  }
};

/*!
 * \brief Template specialization of Str2T<> interface for unsigned 32-bit integer
 */
template<>
class Str2T<uint32_t> {
 public:
  /*!
   * \brief Convert a string into unsigned 32-bit integer
   * \param begin Beginning of the string to convert
   * \param end End of the string to convert
   * \return Converted value, as unsigned 32-bit integer
   */
  static inline uint32_t get(const char* begin, const char* end) {
    return ParseUnsignedInt<uint32_t>(begin, NULL, 10);
  }
};

/*!
 * \brief Template specialization of Str2T<> interface for signed 64-bit integer
 */
template<>
class Str2T<int64_t> {
 public:
  /*!
   * \brief Convert a string into signed 64-bit integer
   * \param begin Beginning of the string to convert
   * \param end End of the string to convert
   * \return Converted value, as signed 64-bit integer
   */
  static inline int64_t get(const char * begin, const char * end) {
    return ParseSignedInt<int64_t>(begin, NULL, 10);
  }
};

/*!
 * \brief Template specialization of Str2T<> interface for unsigned 64-bit integer
 */
template<>
class Str2T<uint64_t> {
 public:
  /*!
   * \brief Convert a string into unsigned 64-bit integer
   * \param begin Beginning of the string to convert
   * \param end End of the string to convert
   * \return Converted value, as unsigned 64-bit integer
   */
  static inline uint64_t get(const char * begin, const char * end) {
    return ParseUnsignedInt<uint64_t>(begin, NULL, 10);
  }
};

/*!
 * \brief Template specialization of Str2T<> interface for float type
 */
template<>
class Str2T<float> {
 public:
  /*!
   * \brief Convert a string into float
   * \param begin Beginning of the string to convert
   * \param end End of the string to convert
   * \return Converted value, in float type
   */
  static inline float get(const char * begin, const char * end) {
    return atof(begin);
  }
};

/*!
 * \brief Template specialization of Str2T<> interface for double type
 */
template<>
class Str2T<double> {
 public:
  /*!
   * \brief Convert a string into double
   * \param begin Beginning of the string to convert
   * \param end End of the string to convert
   * \return Converted value, in double type
   */
  static inline double get(const char * begin, const char * end) {
    return strtod(begin, 0);
  }
};

/*!
 * \brief Parse colon seperated pair v1[:v2]
 * \param begin pointer to string
 * \param end one past end of string
 * \param endptr After conversion, will be set to one past of parsed string
 * \param v1 first value in the pair
 * \param v2 second value in the pair
 * \return number of values parsed
 * \tparam T1 type of v1
 * \tparam T2 type of v2
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

/*!
 * \brief Parse colon seperated triple v1:v2[:v3]
 * \param begin pointer to string
 * \param end one past end of string
 * \param endptr After conversion, will be set to one past of parsed string
 * \param v1 first value in the triple
 * \param v2 second value in the triple
 * \param v3 third value in the triple
 * \return number of values parsed
 * \tparam T1 type of v1
 * \tparam T2 type of v2
 * \tparam T3 type of v3
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
