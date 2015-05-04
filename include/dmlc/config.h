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
  explicit Config(std::istream& is);
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

 private:
  std::map<std::string, std::string> config_map_;
};

} // namespace dmlc

#endif // DMLC_CONFIG_H_
