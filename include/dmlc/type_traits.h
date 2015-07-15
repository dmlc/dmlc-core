/*!
 *  Copyright (c) 2015 by Contributors
 * \file type_traits.h
 * \brief type traits information header
 */
#ifndef DMLC_TYPE_TRAITS_H_
#define DMLC_TYPE_TRAITS_H_

#include "./base.h"
#if DMLC_USE_CXX11
#include <type_traits>
#endif

namespace dmlc {
/*!
 * \brief whether a type is pod type
 * \tparam T the type to query
 */
template<typename T>
struct is_pod {
#if DMLC_USE_CXX11
  /*! \brief the value of the traits */
  static const bool value = std::is_pod<T>::value;
#else
  /*! \brief the value of the traits */
  static const bool value = false;
#endif
};

/*!
 * \brief whether a type have save/load function
 * \tparam T the type to query
 */
template<typename T>
struct has_saveload {
  /*! \brief the value of the traits */
  static const bool value = false;
};

/*! \brief macro to quickly declare traits information */
#define DMLC_DECLARE_TRAITS(Trait, Type, Value)       \
  template<>                                          \
  struct Trait<Type> {                                \
    static const bool value = Value;                  \
  }

//! \cond Doxygen_Suppress
// declare special traits when C++11 is not available
#if DMLC_USE_CXX11 == 0
DMLC_DECLARE_TRAITS(is_pod, int8_t, true);
DMLC_DECLARE_TRAITS(is_pod, int16_t, true);
DMLC_DECLARE_TRAITS(is_pod, int32_t, true);
DMLC_DECLARE_TRAITS(is_pod, int64_t, true);
DMLC_DECLARE_TRAITS(is_pod, uint8_t, true);
DMLC_DECLARE_TRAITS(is_pod, uint16_t, true);
DMLC_DECLARE_TRAITS(is_pod, uint32_t, true);
DMLC_DECLARE_TRAITS(is_pod, uint64_t, true);
DMLC_DECLARE_TRAITS(is_pod, float, true);
DMLC_DECLARE_TRAITS(is_pod, double, true);
#endif

//! \endcond
}  // namespace dmlc
#endif  // DMLC_TYPE_TRAITS_H_
