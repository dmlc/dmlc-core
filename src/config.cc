#include <sstream>
#include <exception>

#include "dmlc/config.h"
#include "dmlc/logging.h"

using namespace std;

namespace dmlc {



Config::Config(istream& is) {
  LoadFromStream(is);
}

void Config::Clear() {
  config_map_.clear();
}

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
  bool GetNextToken(string& tok) {
    // token is defined as
    // 1. [^\s=]+
    // 2. "[(^"|\\")]*"
    // 3. =
    state_ = ParseState::kNone;
    tok.clear();
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
        ParseString(tok);
        state_ = ParseState::kFinish;
        break;
      case '=':
        if(state_ != ParseState::kToken) {
          tok = '=';
          EatChar();
        }
        state_ = ParseState::kFinish;
        break;
      case '#':
        ParseComments();
        break;
      default:
        state_ = ParseState::kToken;
        tok += ch;
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

void Config::LoadFromStream(istream& is) {
  Tokenizer tokenizer(is);
  string kstr, eqstr, vstr;
  try {
    while( true ) {
      tokenizer.GetNextToken(kstr);
      if(kstr.length() == 0) {
        break; // no content left
      }
      tokenizer.GetNextToken(eqstr);
      tokenizer.GetNextToken(vstr);
      if(eqstr != "=") {
        LOG(ERROR) << "Parsing error: expect format \"k = v\"; but got \""
          << kstr << eqstr << vstr << "\"";
      }
      cout << kstr << " " << eqstr << " " << vstr << endl;
    }
  } catch(TokenizeError& err) {
    LOG(ERROR) << "Tokenize error: " << err.what();
  }
  /*string line;
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
  }*/
}

void Config::SetParam(const string& key, const string& value) {
  config_map_[key] = value;
}

const string& Config::GetParam(const string& key) const {
  CHECK_NE(config_map_.find(key), config_map_.end()) << "key \"" << key << "\" not found in configure";
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
