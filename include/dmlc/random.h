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
/*! \brief random shuffle based on c++11 */
class Shuffler {
 public:
  Shuffler(void) {
    std::random_device rd;
    this->Seed(rd());
  }
  explicit Shuffler(unsigned seed) {
    this->Seed(seed);
  }
  /*!
   * \brief seed random number
   * \param seed the random number seed
   */
  inline void Seed(unsigned seed) {
    this->rseed_ = seed;
    this->rengine_ = std::mt19937(seed);
  }
  /*!\brief random shuffle data in */
  template<typename T>
  inline void Shuffle(std::vector<T> *data) {
    std::shuffle(data->begin(), data->end(), rengine_);
  }

 private:
  unsigned rseed_;
  std::mt19937 rengine_;
};
/*! 
 * \brief int uniform sampler based on c++11
 *        P(i/a,b) = 1 / (b - a + 1) 
 */
template< class IntType = int >
class UniformIntSampler {
 public:
  /*!
   * \brief constructor without seed
   * \param min value a
   * \param max value b
   */
  UniformIntSampler(IntType a, IntType b) :
      dis(std::uniform_int_distribution<IntType>(a, b)) {
    std::random_device rd;
    this->rengine_ = std::mt19937(rd());
  }
  /*!
   * \brief constructor with seed
   */ 
  UniformIntSampler(IntType a, IntType b, unsigned seed) :
      dis(std::uniform_int_distribution<IntType>(a, b)) {
    this->rengine_ = std::mt19937(seed);
  }
  inline IntType Get() {
    return dis(rengine_);
  }

 private:
  std::mt19937 rengine_;
  std::uniform_int_distribution<> dis;
};
/*! 
 * \brief float uniform sampler based on c++11
 *        P(i/a,b) = 1 / (b - a) 
 */
template< class RealType = double >
class UniformRealSampler {
 public:
  /*!
   * \brief constructor without seed
   * \param min value a
   * \param max value b
   */
  UniformRealSampler(RealType a, RealType b) :
      dis(std::uniform_real_distribution<RealType>(a, b)) {
    std::random_device rd;
    this->rengine_ = std::mt19937(rd());
  }
  /*!
   * \brief constructor with seed
   */ 
  UniformRealSampler(RealType a, RealType b, unsigned seed) :
      dis(std::uniform_real_distribution<RealType>(a, b)) {
    this->rengine_ = std::mt19937(seed);
  }
  inline RealType Get() {
    return dis(rengine_);
  }

 private:
  std::mt19937 rengine_;
  std::uniform_real_distribution<> dis;
};
/*! 
 * \brief float gaussian sampler based on c++11
 *        P(x/mean,stddev)
 */
template< class RealType = double >
class GaussianSampler {
 public:
  /*!
   * \brief constructor without seed
   * \param mean value
   * \param standard deviation stddev 
   */
  GaussianSampler(RealType mean, RealType stddev) :
      dis(std::normal_distribution<RealType>(mean, stddev)) {
    std::random_device rd;
    this->rengine_ = std::mt19937(rd());
  }
  /*!
   * \brief constructor with seed
   */ 
  GaussianSampler(RealType mean, RealType stddev, unsigned seed) :
      dis(std::normal_distribution<RealType>(mean, stddev)) {
    this->rengine_ = std::mt19937(seed);
  }
  inline RealType Get() {
    return dis(rengine_);
  }

 private:
  std::mt19937 rengine_;
  std::normal_distribution<> dis;
};
}  // namespace dmlc
#endif  // DMLC_RANDOM_H_
