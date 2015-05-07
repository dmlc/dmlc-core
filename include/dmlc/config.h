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

/*!
 * \brief class for config parser
 */
class Config {
 public:
  typedef std::map<std::string, std::string>::const_iterator ConfigIterator;
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
   */
  void SetParam(const std::string& key, const std::string& value);

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
  std::map<std::string, std::string> config_map_;
};

} // namespace dmlc

#endif // DMLC_CONFIG_H_
