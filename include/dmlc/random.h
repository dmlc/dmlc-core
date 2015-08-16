/*!
 *  Copyright (c) 2015 by Contributors
 * \file random.h
 * \brief wrapper for some random utils based on c++11 random
 */
#ifndef DMLC_RANDOM_H_
#define DMLC_RANDOM_H_
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <limits>

/*! \brief namespace for dmlc */
namespace dmlc {
/*! \brief random sampler based on c++11 */
class RandomSampler {
 public:
  /*!
   * \brief constructor without seed
   */ 
  RandomSampler(void) {
    std::random_device rd;
    this->rengine_ = std::mt19937(rd());
  }
  /*!
   * \brief constructor with seed
   * \param seed the random number seed
   */ 
  explicit RandomSampler(unsigned seed) {
    this->rengine_ = std::mt19937(seed);
  }
  /*!
   * \brief random shuffle data in 
   * \param data vector to be shuffled
   */
  template<typename T>
  inline void Shuffle(std::vector<T> *data) {
    std::shuffle(data->begin(), data->end(), rengine_);
  }
  /*!
   * \brief get an uint32 less than n based on uniform distribution
   * \param n maximum number
   * \return random number required
   */
  inline uint32_t NextUInt32(uint32_t n) {
    return GetUniformInt<uint32_t>(0, n);
  }
  /*!
   * \brief get an integer from a to b based on uniform distribution
   * \param a minimum value
   * \param b maximum value
   * \return random number required
   */
  template< class IntType = int >
  inline IntType GetUniformInt(IntType a, IntType b) {
    std::uniform_int_distribution<IntType> dis(a, b);
    return dis(rengine_);
  }
  /*!
   * \brief get a double less between 0 and 1 based on uniform distribution
   * \return random number required
   */
  inline double NextDouble() {
    return GetUniformReal<>(0.0, 1.0);
  }
  /*!
   * \brief get an real number from a to b based on uniform distribution
   * \param a minimum value
   * \param b maximum value
   * \return random number required
   */
  template< class RealType = double >
  inline RealType GetUniformReal(RealType a, RealType b) {
    std::uniform_real_distribution<RealType> dis(a, b);
    return dis(rengine_);
  }
  /*!
   * \brief get an real number based on gaussian distribution
   * \param mean mean value
   * \param stddev standard deviation
   * \return random number required
   */
  template< class RealType = double >
  inline RealType GetGaussian(RealType mean, RealType stddev) {
    std::normal_distribution<RealType> dis(mean, stddev);
    return dis(rengine_);
  }

 private:
  unsigned rseed_;
  std::mt19937 rengine_;
};
}  // namespace dmlc
#endif  // DMLC_RANDOM_H_
