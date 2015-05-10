#include <sstream>
#include <exception>

#include "dmlc/config.h"
#include "dmlc/logging.h"

using namespace std;

namespace dmlc {

namespace details {

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

inline string MakeProtoStringValue(const std::string& str) {
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

} // namespace details

template< template<class, class, class, class> class M>
Config<M>::Config(istream& is) {
  LoadFromStream(is);
}

template< template<class, class, class, class> class M>
void Config<M>::Clear() {
  config_map_.clear();
}

template< template<class, class, class, class> class M>
void Config<M>::LoadFromStream(istream& is) {
  using namespace details;
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
      config_map_.insert(make_pair(key.buf, value.buf));
      is_string_map_.insert(make_pair(key.buf, value.is_string));
    }
  } catch(TokenizeError& err) {
    LOG(ERROR) << "Tokenize error: " << err.what();
  }
}

template< template<class, class, class, class> class M>
void Config<M>::SetParam(const string& key, const string& value) {
  config_map_.insert(make_pair(key, value));
}

template< template<class, class, class, class> class M>
const string& Config<M>::GetParam(const string& key) const {
  CHECK_NE(config_map_.find(key), config_map_.end()) << "key \"" << key << "\" not found in configure";
  return config_map_.find(key)->second;
}

template< template<class, class, class, class> class M>
string Config<M>::ToProtoString(void) const {
  using namespace details;
  ostringstream oss;
  for(ConfigIterator iter = begin(); iter != end(); ++iter) {
    const ConfigEntry& entry = *iter;
    bool is_string = is_string_map_.find(entry.first)->second;
    oss << entry.first << " : " <<
      (is_string? MakeProtoStringValue(entry.second) : entry.second)
      << "\n";
  }
  return oss.str();
}
  
} // namespace dmlc
