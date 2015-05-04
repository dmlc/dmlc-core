#include "dmlc/config.h"

namespace dmlc {

Config::Config(std::istream& is) {
  std::string line;
  while(std::getline(is, line)) {
    size_t pos = line.find_first_of('=');
    size_t key_st = 0, key_ed = pos;
    size_t val_st = pos + 1, val_ed = line.length();
    // for value with quotation, get rid of quotations.
    if(line[val_st] = "\"") {
      ++val_st;
      --val_ed;
    }
    config_map_[line.substr(key_st, key_ed)] = line.substr(val_st, val_ed);
  }
}

void Config::SetParam(const std::string& key, const std::string& value) {
  config_map_[key] = value;
}

const std::string& Config::GetParam(const std::string& key) const {
  return config_map_.find(key)->second;
}

std::string Config::ToProtoString(void) const {
}

} // namespace dmlc
