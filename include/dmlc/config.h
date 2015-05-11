/*!
 * Copyright (c) 2015 by Contributors
 * \file config.h
 * \brief defines config parser class
 */
#ifndef DMLC_CONFIG_H_
#define DMLC_CONFIG_H_

#include <iostream>
#include <cstring>
#include <iterator>
#include <map>
#include <vector>
#include <sstream>

/*! \brief namespace for dmlc */
namespace dmlc {

/*!
 * \brief class for config parser
 */
class Config {

 public:
  typedef std::pair<std::string, std::string> ConfigEntry;
  class ConfigIterator;

  /*!
   * \brief create empty config
   * \param whether the config supports multi value
   */
  Config(bool multi_value = false);
  /*!
   * \brief create config and load content from the given stream
   * \param input stream
   * \param whether the config supports multi value
   */
  explicit Config(std::istream& is, bool multi_value = false);
  /*!
   * \brief clear all the values
   */
  void Clear(void);
  /*!
   * \brief load the contents from the stream
   * \param is the stream as input
   */
  void LoadFromStream(std::istream& is);
  /*!
   * \brief set a key-value pair into the config
   * \param key key
   * \param value value
   * \param is_string whether the value should be wrapped by quotes in proto string
   */
  template<class T>
  void SetParam(const std::string& key, const T& value, bool is_string = false);

  /*!
   * \brief get the config under the key
   * \param key key
   * \return config value
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
  ConfigIterator begin() const;

  /*!
   * \brief get end iterator
   * \return end iterator
   */
  ConfigIterator end() const;

 private:
  struct ConfigValue {
    std::string val;
    bool is_string;
  };
  typedef std::map<std::string, std::vector<ConfigValue>> InternalMap;
  typedef typename InternalMap::const_iterator InternalMapIterator;
  void Insert(const std::string& key, const std::string& value, bool is_string);

 public:

  class ConfigIterator : public std::iterator< std::input_iterator_tag, ConfigEntry > {
   public:
    ConfigIterator(InternalMapIterator ki, size_t val_index);
    ConfigIterator(const ConfigIterator& other);
    ConfigIterator& operator ++ ();
    ConfigIterator operator ++ (int);
    bool operator == (const ConfigIterator& rhs) const;
    bool operator != (const ConfigIterator& rhs) const;
    ConfigEntry operator * ();
   private:
    InternalMapIterator key_iter_;
    size_t value_index_;
  };

 private:
  InternalMap config_map_;
  bool multi_value_;

};

template<class T>
void Config::SetParam(const std::string& key, const T& value, bool is_string) {
  std::ostringstream oss;
  oss << value;
  Insert(key, oss.str(), is_string);
}

} // namespace dmlc

#endif // DMLC_CONFIG_H_
