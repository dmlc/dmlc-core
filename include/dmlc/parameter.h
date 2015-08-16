/*!
 * Copyright (c) 2015 by Contributors
 * \file parameter.h
 * \brief Provide lightweight util to do parameter setup and checking.
 */
#ifndef DMLC_PARAMETER_H_
#define DMLC_PARAMETER_H_

#include <cstddef>
#include <sstream>
#include <limits>
#include <map>
#include <set>
#include <typeinfo>
#include <string>
#include <vector>
#include "./base.h"
#include "./logging.h"
#include "./type_traits.h"

namespace dmlc {
// this file is backward compatible with non-c++11
/*! \brief Error throwed by parameter checking */
struct ParamError : public dmlc::Error {
  /*!
   * \brief constructor
   * \param msg error message
   */
  explicit ParamError(const std::string &msg)
      : dmlc::Error(msg) {}
};

/*! \brief internal namespace for parameter manangement */
namespace parameter {
// forward declare ParamManager
class ParamManager;
// forward declare FieldAccessEntry
class FieldAccessEntry;
// forward declare FieldEntry
template<typename DType>
struct FieldEntry;
}  // namespace parameter

/*!
 * \brief Parameter is the base type every parameter struct should inheritate from
 * The following code is a complete example to setup parameters.
 * \code
 *   struct Param : public dmlc::Parameter<Param> {
 *     float learning_rate;
 *     int num_hidden;
 *     std::string name;
 *     // declare parameters in header file
 *     DMLC_DECLARE_PARAMETER(Param) {
 *       DMLC_DECLARE_FIELD(num_hidden).set_range(0, 1000);
 *       DMLC_DECLARE_FIELD(learning_rate).set_default(0.01f);
 *       DMLC_DECLARE_FIELD(name).set_default("hello");
 *     }
 *   };
 *   // register it in cc file
 *   DMLC_REGISTER_PARAMETER(Param);
 * \endcode
 *
 *  After that, the Param struct will get all the functions defined in Parameter.
 * \tparam PType the type of parameter struct
 *
 * \sa DMLC_DECLARE_FIELD, DMLC_REGISTER_PARAMETER, DMLC_DECLARE_PARAMETER
 */
template<typename PType>
struct Parameter {
 public:
  /*!
   * \brief initialize the parameter by keyword arguments.
   *  This function will initialize the parameter struct, check consistency
   *  and throw error if something wrong happens.
   *
   * \param kwargs map of keyword arguments
   * \throw ParamError when something go wrong.
   */
  inline void Init(const std::map<std::string, std::string> &kwargs) {
    PType::__MANAGER__()->Set(static_cast<PType*>(this), kwargs);
  }

 protected:
  /*!
   * \brief internal function to allow declare of a parameter memember
   * \param key the key name of the parameter
   * \param ref the reference to the parameter in the struct.
   */
  template<typename DType>
  inline parameter::FieldEntry<DType>& DECLARE(const std::string &key, DType &ref) { // NOLINT(*)
    parameter::FieldEntry<DType> *e =
        new parameter::FieldEntry<DType>();
    e->Init(key, static_cast<PType*>(this), ref);
    PType::__MANAGER__()->AddEntry(key, e);
    return *e;
  }
};

//! \cond Doxygen_Suppress
/*!
 * \brief macro used to declare parameter
 *
 * Example:
 * \code
 *   struct Param : public dmlc::Parameter<Param> {
 *     // declare parameters in header file
 *     DMLC_DECLARE_PARAMETER(Param) {
 *        // details of declarations
 *     }
 *   };
 * \endcode
 *
 * This macro need to be put in a source file so that registeration only happens once.
 * Refer to example code in Parameter for details
 * \param PType the name of parameter struct.
 * \sa Parameter
 */
#define DMLC_DECLARE_PARAMETER(PType)                      \
  static ::dmlc::parameter::ParamManager *__MANAGER__();   \
  inline void __DECLARE__()
/*!
 * \brief macro to declare fields
 * \param FieldName the name of the field.
 */
#define DMLC_DECLARE_FIELD(FieldName)  this->DECLARE(#FieldName, FieldName)
/*!
 * \brief Macro used to register parameter.
 *
 * This macro need to be put in a source file so that registeration only happens once.
 * Refer to example code in Parameter for details
 * \param PType the type of parameter struct.
 * \sa Parameter
 */
#define DMLC_REGISTER_PARAMETER(PType)                                  \
  ::dmlc::parameter::ParamManager *PType::__MANAGER__() {               \
    static ::dmlc::parameter::ParamManager inst;                        \
    return &inst;                                                       \
  }                                                                     \
  static ::dmlc::parameter::ParamManager *__make__ ## PType ## ParamManager__ = \
      PType::__MANAGER__()->__INIT__<PType>(#PType);

//! \endcond
/*!
 * \brief internal namespace for parameter manangement
 * There is no need to use it directly in normal case
 */
namespace parameter {
/*!
 * \brief FieldAccessEntry interface to help manage the parameters
 *  Each entry can be used to access one parameter in the Parameter struct.
 *
 *  This is an internal interface used that is used to manage parameters
 */
class FieldAccessEntry {
 public:
  FieldAccessEntry()
      : has_default_(false) {}
  /*! \brief destructor */
  virtual ~FieldAccessEntry() {}
  /*!
   * \brief set the default value, throw error if no default is presented
   * \param head the pointer to the head of the struct
   */
  virtual void SetDefault(void *head) = 0;
  /*!
   * \brief set the parameter by string value
   * \param head the pointer to the head of the struct
   * \param value the value to be set
   */
  virtual void Set(void *head, const std::string &value) = 0;
  // check if value is OK
  virtual void Check(void *head) const {}
  /*!
   * \brief get string representation of the value
   * \param head pointer to head of parameter struct
   * \return string representation
   */
  virtual std::string GetValueString(void *head) const = 0;
  /*! \return the key of the parameter */
  inline const std::string &key() const {
    return key_;
  }
  /*! \return the type of the parameter */
  inline const std::string &type() const {
    return type_;
  }
  /*! \return whether the parameter has default value */
  inline bool has_default() const {
    return has_default_;
  }

 protected:
  /*! \brief whether this parameter have default value */
  bool has_default_;
  /*! \brief positional index of parameter in struct */
  size_t index_;
  /*! \brief parameter key name */
  std::string key_;
  /*! \brief parameter type */
  std::string type_;
  /*! \brief internal parser */
  std::istringstream is_;
  // allow ParamManager to modify self
  friend class ParamManager;
};
/*!
 * \brief manager class to handle parameter setting for each type
 *  An manager will be created for each parameter types.
 */
class ParamManager {
 public:
  /*! \brief destructor */
  ~ParamManager() {
    for (size_t i = 0; i < entry_.size(); ++i) {
      delete entry_[i];
    }
  }
  /*!
   * \brief find the access entry by parameter key
   * \param key the key of the parameter.
   * \return pointer to FieldAccessEntry, NULL if nothing is found.
   */
  inline FieldAccessEntry *Find(const std::string &key) const {
    std::map<std::string, FieldAccessEntry*>::const_iterator it =
        entry_map_.find(key);
    if (it == entry_map_.end()) return NULL;
    return it->second;
  }
  /*!
   * \brief set parameter by keyword arguments.
   * \param head head to the parameter field.
   * \param kwargs the keyword arguments of parameters.
   */
  inline void Set(void *head, const std::map<std::string, std::string> &kwargs) const {
    for (std::map<std::string, std::string>::const_iterator it = kwargs.begin();
         it != kwargs.end(); ++it) {
      FieldAccessEntry *e = Find(it->first);
      if (e != NULL) {
        e->Set(head, it->second);
      } else {
        std::ostringstream os;
        os << "Cannot find parameter " << it->first;
        throw dmlc::ParamError(os.str());
      }
    }

    for (std::map<std::string, FieldAccessEntry*>::const_iterator it = entry_map_.begin();
         it != entry_map_.end(); ++it) {
      if (kwargs.count(it->first) == 0) {
        it->second->SetDefault(head);
      }
    }
  }
  /*!
   * \brief internal function to add entry to manager,
   *  The manager will take ownership of the entry.
   * \param key the key to the parameters
   * \param e the pointer to the new entry.
   */
  inline void AddEntry(const std::string &key, FieldAccessEntry *e) {
    e->index_ = entry_.size();
    entry_.push_back(e);
    // TODO(bing) better error message
    if (entry_map_.count(key) != 0) {
      LOG(FATAL) << "key " << key << " has already been registered in " << param_name_;
    }
    entry_map_[key] = e;
  }
  /*!
   * \brief internal function to initialize parameter entry.
   *  This function will only by call once during DMLC_REGISTER_PARAMETER
   */
  template<typename PType>
  inline ParamManager *__INIT__(const std::string &param_name) {
    if (param_name_.length() != 0) {
      LOG(FATAL) << "DMLC_REGISTER_PARAMETER(" << param_name << ") has already been called";
    }
    PType p;
    p.__DECLARE__();
    param_name_ = param_name;
    return this;
  }

 private:
  /*! \brief parameter struct name */
  std::string param_name_;
  /*! \brief positional list of entries */
  std::vector<FieldAccessEntry*> entry_;
  /*! \brief map of key to entry */
  std::map<std::string, FieldAccessEntry*> entry_map_;
};

//! \cond Doxygen_Suppress
// The following piece of code will be template heavy and less documented

// Base class of FieldEntry
// implement set_default
template<typename TEntry, typename DType>
class FieldEntryBase : public FieldAccessEntry {
 public:
  // entry type
  typedef TEntry EntryType;
  // implement set value
  virtual void Set(void *head, const std::string &value) {
    is_.str(value);
    is_ >> this->Get(head);
    if (is_.fail() || !is_.eof()) {
      std::ostringstream os;
      os << "Invalid Parameter format for " << key_
         << " expect " << type_ << " but value=\'" << value<< '\'';
      throw dmlc::ParamError(os.str());
    }
    this->Check(head);
  }
  // implement value to string
  virtual std::string GetValueString(void *head) const {
    std::ostringstream os;
    os << Get(head);
    return os.str();
  }
  // implement set head to default value
  virtual void SetDefault(void *head) {
    if (!has_default_) {
      std::ostringstream os;
      os << "Parameter " << key_
         << " is not presented";
      throw dmlc::ParamError(os.str());
    } else {
      this->Get(head) = default_value_;
    }
  }
  // return reference of self as derived type
  inline TEntry &self() {
    return *(static_cast<TEntry*>(this));
  }
  // implement set_default
  inline TEntry &set_default(const DType &default_value) {
    default_value_ = default_value;
    has_default_ = true;
    // return self to allow chaining
    return this->self();
  }
  // initialization function
  inline void Init(const std::string &key,
                   void *head, DType &ref) { // NOLINT(*)
    this->key_ = key;
    if (this->type_.length() == 0) {
      this->type_ = dmlc::type_name<DType>();
    }
    this->offset_ = ((char*)&ref) - ((char*)head);  // NOLINT(*)
  }

 protected:
  // get the internal representation of parameter
  // for example if this entry corresponds field param.learning_rate
  // then Get(&param) will return reference to param.learning_rate
  inline DType &Get(void *head) const {
    return *(DType*)((char*)(head) + offset_);  // NOLINT(*)
  }
  // internal offset of the field
  ptrdiff_t offset_;
  // default value of field
  DType default_value_;
};

// parameter base for numeric types that have range
template<typename TEntry, typename DType>
class FieldEntryNumeric
    : public FieldEntryBase<TEntry, DType> {
 public:
  FieldEntryNumeric() {
    this->end_ = std::numeric_limits<DType>::max();
    if (std::numeric_limits<DType>::is_integer) {
      this->begin_ = std::numeric_limits<DType>::min();
    } else {
      // use neg max for floating pts
      this->begin_ = -std::numeric_limits<DType>::max();
    }
  }
  // implement set_range
  inline TEntry &set_range(DType begin, DType end) {
    begin_ = begin; end_ = end;
    return this->self();
  }
  // consistency check for numeric ranges
  virtual void Check(void *head) const {
    FieldEntryBase<TEntry, DType>::Check(head);
    int v = this->Get(head);
    if (v < begin_ || v >= end_) {
      std::ostringstream os;
      os << "value " << v << "for Parameter " << this->key_
         << "exceed bound [" << begin_ << ',' << end_ <<')';
      throw dmlc::ParamError(os.str());
    }
  }

 protected:
  // data bound
  DType begin_, end_;
};

/*!
 * \brief FieldEntry defines parsing and checking behavior of DType.
 * This class can be specialized to implement specific behavior of more settings.
 * \tparam DType the data type of the entry.
 */
template<typename DType>
class FieldEntry :
      public IfThenElseType<dmlc::is_arithmetic<DType>::value,
                            FieldEntryNumeric<FieldEntry<DType>, DType>,
                            FieldEntryBase<FieldEntry<DType>, DType> >::Type {
};

// specialize define for string
template<>
class FieldEntry<std::string>
    : public FieldEntryBase<FieldEntry<std::string>, std::string> {
 public:
  // parent class
  typedef FieldEntryBase<FieldEntry<std::string>, std::string> Parent;
  // override check
  virtual void Check(void *head) const {
    Parent::Check(head);
    std::string value = this->Get(head);
    if (enum_set_.size() != 0 && enum_set_.count(value) == 0) {
      throw dmlc::ParamError("value not found in enum");
    }
  }
  // add new set function
  inline FieldEntry<std::string> &add_enum(const std::string &value) {
    enum_set_.insert(value);
    return this->self();
  }

 private:
  // enumeration set in enum mode
  std::set<std::string> enum_set_;
};

}  // namespace parameter
//! \endcond
}  // namespace dmlc
#endif  // DMLC_PARAMETER_H_
