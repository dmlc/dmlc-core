/*!
 *  Copyright (c) 2015 by Contributors
 * \file data.h
 * \brief defines common input data structure,
 *  and interface for handling the input data
 */
#ifndef DMLC_DATA_H_
#define DMLC_DATA_H_
#include <string>
#include "./base.h"
#include "./io.h"
#include "./logging.h"

namespace dmlc {
/*!
 * \brief this defines the float point
 * that will be used to store feature values
 */
typedef float real_t;

// This file describes common data structure that can be used
// for large-scale machine learning, this may not be a complete list
// But we will keep the most common and useful ones, and keep adding new ones
/*!
 * \brief data iterator interface
 *  this is not a C++ style iterator, but nice for data pulling:)
 *  This interface is used to pull in the data
 *  The system can do some useful tricks for you like pre-fetching
 *  from disk and pre-computation.
 *
 * Usage example:
 *   itr->BeforeFirst();
 *   while (itr->Next()) {
 *      const DType &batch = itr->Value();
 *      // some computations
 *   }
 *
 * \tparam DType the data type 
 */
template<typename DType>
class IDataIter {
 public:
  /*! \brief destructor */
  virtual ~IDataIter(void) {}
  /*!
   * \brief set the parameter
   * \param name name of parameter
   * \param val value of parameter
   */
  virtual void SetParam(const char *name, const char *val) {}
  /*!
   * \brief initalize the iterator,
   * this is called before everything
   */
  virtual void Init(void) {}
  /*! \brief set before first of the item */
  virtual void BeforeFirst(void) = 0;
  /*! \brief move to next item */
  virtual bool Next(void) = 0;
  /*! \brief get current data */
  virtual const DType &Value(void) const = 0;
};
/*!
 * \brief a batch of data, containing several rows in sparse matrix 
 *  This is useful for (streaming-style) algorithms that scans through rows of data
 *  examples include: SGD, GD, L-BFGS, kmeans
 *  
 *  The size of batch is usually large enough so that parallelizing over the rows
 *  can give significant speedup
 * \tparam IndexType type to store the index used in row batch
 */
template<typename IndexType>
class RowBatch {
 public:
  /*! \brief a specific instance in row batch */
  struct Inst {
    /*! \brief label of the instance */
    real_t label;
    /*! \brief length of the sparse vector */
    size_t length;
    /*! \brief index of each instance */
    const IndexType *index_;
    /*! \brief array value of each instance, this can be NULL */
    const real_t *value_;
    /*!
     * \brief get i-th index in the instance
     * \param i position
     * \return i-th index
     */
    inline IndexType index(size_t i) const {
      return index_[i];
    }
    /*!
     * \brief get i-th value in the instance
     * \param i position
     * \return i-th value
     */
    inline real_t value(size_t i) const {
      return value_ == NULL ? static_cast<real_t>(1) : value_[i];
    }
    /*!
     * \brief helper function to compute dot product of current
     * \param weight the dense array of weight we want to product
     * \parma size the size of the weight vector
     * \tparam V type of the weight vector
     */
    template<typename V>
    inline V SDot(const V *weight, size_t size) const {
      V sum = static_cast<V>(0);
      if (value == NULL) {
        for (size_t i = 0; i < length; ++i) {
          CHECK(index[i] < size) << "feature index exceed bound";
          sum += weight[index[i]];
        }
      } else {
        for (size_t i = 0; i < length; ++i) {
          CHECK(index[i] < size) << "feature index exceed bound";
          sum += weight[index[i]] * value[i];
        }
      }
    }
  };
  /*! \brief batch size */
  size_t size;
  /*! \brief array[size+1], row pointer to beginning of each rows */
  const size_t *ind_ptr_;
  /*! \brief array[size] label of each instance */
  const real_t *label_;
  /*! \brief feature index */
  const IndexType *index_;
  /*! \brief feature value, can be NULL, indicating all values are 1 */
  const real_t *value_;
  /*!
   * \brief create a new instance of iterator that returns rowbatch
   *  by default, a in-memory based iterator will be returned
   *
   * \param source data source from which we get the data
   * \param cfg additional configs we like to pass, normally can be empty
   * \return the created data iterator
   */
  static IDataIter<RowBatch<IndexType> > *
  CreateIter(InputSplit *source,
             const std::string &cfg = std::string());
  /*!
   * \brief get specific rows in the batch 
   * \param rowid the rowid in that row
   * \return the instance corresponding to the row
   */
  inline Inst operator[](size_t rowid) const {
    CHECK(rowid < size);
    Inst inst;
    inst.label = label_[rowid];
    inst.length = ind_ptr_[rowid + 1] - ind_ptr_[rowid];
    inst.index_ = index_ + ind_ptr_[rowid];
    if (value_ == NULL) {
      inst.value_ = NULL;
    } else {
      inst.value_ = value_ + ind_ptr_[rowid];
    }
    return inst;
  }
};
}  // namespace dmlc
#endif  // DMLC_DATA_H_
