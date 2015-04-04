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
/*!
 * \brief this defines the index type
 * that will be used to store feature index
 */
typedef unsigned index_t;

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
  /*! \brief set before first of the item */
  virtual void BeforeFirst(void) = 0;
  /*! \brief move to next item */
  virtual bool Next(void) = 0;
  /*! \brief get current data */
  virtual const DType &Value(void) const = 0;
};

/*!
 * \brief one row of training instance 
 * \tparam IndexType type of index
 */
template<typename IndexType>
struct Row {
  /*! \brief label of the instance */
  real_t label;
  /*! \brief length of the sparse vector */
  size_t length;
  /*! \brief index of each instance */
  const IndexType *index;
  /*!
   * \brief array value of each instance, this can be NULL
   *  indicating every value is set to be 1
   */
  const real_t *value;
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

/*!
 * \brief a block of data, containing several rows in sparse matrix 
 *  This is useful for (streaming-sxtyle) algorithms that scans through rows of data
 *  examples include: SGD, GD, L-BFGS, kmeans
 *
 *  The size of batch is usually large enough so that parallelizing over the rows
 *  can give significant speedup
 * \tparam IndexType type to store the index used in row batch
 */
template<typename IndexType>
struct RowBlock {
  /*! \brief batch size */
  size_t size;
  /*! \brief array[size+1], row pointer to beginning of each rows */
  const size_t *offset;
  /*! \brief array[size] label of each instance */
  const real_t *label;
  /*! \brief feature index */
  const IndexType *index;
  /*! \brief feature value, can be NULL, indicating all values are 1 */
  const real_t *value;
  /*!
   * \brief get specific rows in the batch 
   * \param rowid the rowid in that row
   * \return the instance corresponding to the row
   */
  inline Row<IndexType> operator[](size_t rowid) const;
  /*!
   * \brief create a new instance of iterator that returns rowbatch
   *  by default, a in-memory based iterator will be returned
   *
   * \param source data source from which we get the data,
   the iterator will takes the ownership of source and caller do not need to free it
   * \param cfg additional configs we like to pass, normally can be empty
   * \return the created data iterator
   */
  static IDataIter<RowBlock<IndexType> > *
  CreateIter(InputSplit *source,
             const std::string &cfg = std::string());
};

// implementation of operator[]
template<typename IndexType>
inline Row<IndexType> RowBlock<IndexType>::operator[](size_t rowid) const {
  CHECK(rowid < size);
  Row<IndexType> inst;
  inst.label = label[rowid];
  inst.length = offset[rowid + 1] - offset[rowid];
  inst.index = index + offset[rowid];
  if (value == NULL) {
    inst.value = NULL;
  } else {
    inst.value = value + offset[rowid];
  }
  return inst;  
}
}  // namespace dmlc
#endif  // DMLC_DATA_H_
