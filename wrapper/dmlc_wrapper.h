#ifndef DMLC_WRAPPER_H_
#define DMLC_WRAPPER_H_

/*!
 * \file DMLC_wrapper.h
 * \author Tianqi Chen
 * \brief a C style wrapper of DMLC
 *  can be used to create wrapper of other languages
 */
#ifdef _MSC_VER
#define DMLC_DLL __declspec(dllexport)
#else
#define DMLC_DLL
#endif
// manually define unsign long
typedef unsigned long rbt_ulong;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}  // C
#endif
#endif  // XGBOOST_WRAPPER_H_
