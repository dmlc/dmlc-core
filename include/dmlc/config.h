/*!
 * Copyright (c) 2015 by Contributors
 * \file config.h
 * \brief defines config parser class
 */
#ifndef DMLC_CONFIG_H_
#define DMLC_CONFIG_H_

#include <iostream>
#include <cstring>
#include <map>

/*! \brief namespace for dmlc */
namespace dmlc {

struct MultiMapInserter;
struct MapInserter;

/*!
 * \brief class for config parser
 */
template< template< class K,
                    class V,
                    class Compare = std::less<K>,
                    class Alloc = std::allocator<std::pair<const K, V>> > class M,
          class Inserter>
class Config {
 public:
  typedef typename M<std::string, std::string>::const_iterator ConfigIterator;
  typedef std::pair<std::string, std::string> ConfigEntry;

 public:
  /*!
   * \brief create empty config
   */
  Config();
  /*!
   * \brief create config and load content from the given stream
   * \param input stream
   */
  explicit Config(std::istream& is);
  /*!
   * \brief clear all the values
   */
  void Clear(void);
  /*!
   * \brief load the contents from the stream
   * \param the stream as input
   */
  void LoadFromStream(std::istream& is);
  /*!
   * \brief set a key-value pair into the config
   * \param key key
   * \param value value
   * \param is_string whether the value should be wrapped by quotation mark when converting to proto string.
   */
  template<class T>
  void SetParam(const std::string& key, const T& value, bool is_string = false);

  /*!
   * \brief get the config under the key
   * \param key key
   * \return config value represneted by string
   */
  const std::string& GetParam(const std::string& key) const;
  /*!
   * \brief transform all the configuration into string recognizable to protobuf
   * \return string that could be parsed directly by protobuf
   */
  std::string ToProtoString(void) const;

  /*!
   * \brief get begin iterator
   * \return begin iterator
   */
  ConfigIterator begin() const { return config_map_.begin(); }

  /*!
   * \brief get end iterator
   * \return end iterator
   */
  ConfigIterator end() const { return config_map_.end(); }

 private:
  void Insert(const std::string& key, const std::string& value, bool is_string);

 private:
  M<std::string, bool> is_string_map_;
  M<std::string, std::string> config_map_;
};

typedef Config<std::map, MapInserter> SimpleConfig;
typedef Config<std::multimap, MultiMapInserter> MultiConfig;

} // namespace dmlc

#include "./config-inl.h"

#endif // DMLC_CONFIG_H_
