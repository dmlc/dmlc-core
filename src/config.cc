#include <sstream>

#include "dmlc/config.h"
#include "dmlc/logging.h"

using namespace std;

namespace dmlc {

Config::Config() {}

Config::Config(istream& is) {
  LoadFromStream(is);
}

void Config::Clear() {
  config_map_.clear();
}

void Config::LoadFromStream(istream& is) {
  string line;
  int lnum = 0;
  while(getline(is, line)) {
    ++lnum;
    size_t pos = line.find_first_of('=');
    if(pos == 0 || pos == string::npos) {
      LOG(WARNING) << "config parsing error on line(" << lnum << "): \"" << line << "\"";
      continue;
    }
    size_t key_st = 0, key_len = pos;
    size_t val_st = pos + 1, val_len = line.length() - pos - 1;
    config_map_[line.substr(key_st, key_len)] = line.substr(val_st, val_len);
  }
}

void Config::SetParam(const string& key, const string& value) {
  config_map_[key] = value;
}

const string& Config::GetParam(const string& key) const {
  CHECK(config_map_.find(key) != config_map_.end()) << "key \"" << key << "\" not found in configure";
  return config_map_.find(key)->second;
}

string Config::ToProtoString(void) const {
  ostringstream oss;
  for(ConfigIterator iter = begin(); iter != end(); ++iter) {
    const ConfigEntry& entry = *iter;
    oss << entry.first << " : " << entry.second << "\n";
  }
  return oss.str();
}

Config::ConfigIterator Config::begin() const {
  return config_map_.begin();
}

Config::ConfigIterator Config::end() const {
  return config_map_.end();
}

} // namespace dmlc
