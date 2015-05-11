#include <sstream>
#include <exception>

#include "dmlc/config.h"
#include "dmlc/logging.h"

using namespace std;

namespace dmlc {

struct Token {
  std::string buf;
  bool is_string;
};

class TokenizeError : public exception {
 public:
  TokenizeError(const string& msg = "tokenize error"): msg_(msg) {
  }
  virtual const char* what() const throw() {
    return msg_.c_str();
  }
 private:
  string msg_;
};

class Tokenizer {
 public:
  Tokenizer(istream& is): is_(is), state_(ParseState::kNone) {}
  bool GetNextToken(Token& tok) {
    // token is defined as
    // 1. [^\s=]+
    // 2. "[(^"|\\")]*"
    // 3. =
    state_ = ParseState::kNone;
    tok.buf.clear();
    tok.is_string = false;
    char ch;
    while( (ch = PeekChar()) != EOF && state_ != ParseState::kFinish ) {
      switch(ch) {
      case ' ': case '\t': case '\n': case '\r':
        if(state_ == ParseState::kToken) {
          state_ = ParseState::kFinish;
        } else {
          EatChar(); // ignore
        }
        break;
      case '\"':
        ParseString(tok.buf);
        state_ = ParseState::kFinish;
        tok.is_string = true;
        break;
      case '=':
        if(state_ != ParseState::kToken) {
          tok.buf = '=';
          EatChar();
        }
        state_ = ParseState::kFinish;
        break;
      case '#':
        ParseComments();
        break;
      default:
        state_ = ParseState::kToken;
        tok.buf += ch;
        EatChar();
        break;
      };
    }
    return PeekChar() != EOF;
  }

  void ParseString(string& tok) {
    EatChar(); // eat the first quotation mark
    char ch;
    while( (ch = PeekChar()) != '\"' ) {
      switch(ch) {
        case '\\':
          EatChar();
          ch = PeekChar();
          if(ch == '\"') {
            tok += '\"';
          } else {
            throw TokenizeError("error parsing escape characters");
          }
          break;
        case '\n': case '\r': case EOF:
          throw TokenizeError("quotation mark is not closed");
        default:
          tok += ch;
          break;
      };
      EatChar();
    }
    EatChar(); // eat the last quotation mark
  };

  void ParseComments() {
    char ch;
    while( (ch = PeekChar()) ) {
      if(ch == '\n' || ch == '\r' || ch == EOF) {
        break; // end of comment
      }
      EatChar(); // ignore all others
    }
  }
  
 private:
  char PeekChar() {
    return is_.peek();
  }
  void EatChar() {
    is_.get();
  }
 private:
  enum class ParseState {
    kNone = 0,
    kToken,
    kFinish,
  };
  istream& is_;
  ParseState state_;
};

//////////////////////// Config /////////////////////////////
Config::Config(bool m): multi_value_(m) {
}

Config::Config(istream& is, bool m): multi_value_(m) {
  LoadFromStream(is);
}

void Config::Clear() {
  config_map_.clear();
}

void Config::LoadFromStream(istream& is) {
  Tokenizer tokenizer(is);
  Token key, eqop, value;
  try {
    while( true ) {
      tokenizer.GetNextToken(key);
      if(key.buf.length() == 0) {
        break; // no content left
      }
      tokenizer.GetNextToken(eqop);
      tokenizer.GetNextToken(value);
      if(eqop.buf != "=") {
        LOG(ERROR) << "Parsing error: expect format \"k = v\"; but got \""
          << key.buf << eqop.buf << value.buf << "\"";
      }
      Insert(key.buf, value.buf, value.is_string);
    }
  } catch(TokenizeError& err) {
    LOG(ERROR) << "Tokenize error: " << err.what();
  }
}

const string& Config::GetParam(const string& key) const {
  CHECK_NE(config_map_.find(key), config_map_.end()) << "key \"" << key << "\" not found in configure";
  return config_map_.find(key)->second[0].val; // only get the first appearence
}

string MakeProtoStringValue(const std::string& str) {
  string rst = "\"";
  for(size_t i = 0; i < str.length(); ++i) {
    if(str[i] != '\"') {
      rst += str[i];
    } else {
      rst += "\\\"";
    }
  }
  rst += "\"";
  return rst;
}

string Config::ToProtoString(void) const {
  ostringstream oss;
  for(const auto& pair : config_map_) {
    for(const ConfigValue& v : pair.second) {
      oss << pair.first << " : " <<
        (v.is_string? MakeProtoStringValue(v.val) : v.val)
        << "\n";
    }
  }
  return oss.str();
}
  
Config::ConfigIterator Config::begin() const {
  return ConfigIterator(config_map_.begin(), 0);
}

Config::ConfigIterator Config::end() const {
  return ConfigIterator(config_map_.end(), 0);
}

void Config::Insert(const std::string& key, const std::string& value, bool is_string) {
  if(!multi_value_) {
    config_map_[key] = std::vector<ConfigValue>();
  }
  config_map_[key].push_back({value, is_string});
}

////////////////////// ConfigIterator //////////////////////

Config::ConfigIterator::ConfigIterator(InternalMapIterator ki, size_t vi): key_iter_(ki), value_index_(vi) {
}

Config::ConfigIterator::ConfigIterator(const Config::ConfigIterator& other): key_iter_(other.key_iter_), value_index_(other.value_index_) {
}

Config::ConfigIterator& Config::ConfigIterator::operator ++ () {
  ++value_index_;
  if(value_index_ == key_iter_->second.size()) {
    ++key_iter_;
    value_index_ = 0;
  }
  return *this;
}

Config::ConfigIterator Config::ConfigIterator::operator ++ (int) {
  ConfigIterator tmp(*this);
  operator++();
  return tmp;
}

bool Config::ConfigIterator::operator == (const Config::ConfigIterator& rhs) const {
  return key_iter_ == rhs.key_iter_ && value_index_ == rhs.value_index_;
}

bool Config::ConfigIterator::operator != (const Config::ConfigIterator& rhs) const {
  return ! (operator == (rhs));
}

Config::ConfigEntry Config::ConfigIterator::operator * () {
  return make_pair(key_iter_->first, key_iter_->second[value_index_].val);
}

} // namespace dmlc
