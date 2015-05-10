#include <sstream>
#include <exception>
#include <type_traits>

#include "dmlc/config.h"
#include "dmlc/logging.h"

namespace dmlc {

namespace details {

struct Token {
  std::string buf;
  bool is_string;
};

class TokenizeError : public std::exception {
 public:
  TokenizeError(const std::string& msg = "tokenize error"): msg_(msg) {
  }
  virtual const char* what() const throw() {
    return msg_.c_str();
  }
 private:
  std::string msg_;
};

class Tokenizer {
 public:
  Tokenizer(std::istream& is): is_(is), state_(ParseState::kNone) {}
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

  void ParseString(std::string& tok) {
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
  std::istream& is_;
  ParseState state_;
};

inline std::string MakeProtoStringValue(const std::string& str) {
  std::string rst = "\"";
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

struct MapInserter {
  template<class K, class V>
  static void Insert(std::map<K, V>& m, const std::pair<K, V>& p) {
    m[p.first] = p.second;
  }
};

struct MultiMapInserter {
  template<class K, class V>
  static void Insert(std::multimap<K, V>& m, const std::pair<K, V>& p) {
    m.insert(p);
  }
};

template< template<class, class, class, class> class M, class I>
Config<M, I>::Config() {
}

template< template<class, class, class, class> class M, class I>
Config<M, I>::Config(std::istream& is) {
  LoadFromStream(is);
}

template< template<class, class, class, class> class M, class I>
void Config<M, I>::Clear() {
  config_map_.clear();
}

template< template<class, class, class, class> class M, class I>
void Config<M, I>::LoadFromStream(std::istream& is) {
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
      Insert(key.buf, value.buf, value.is_string);
    }
  } catch(TokenizeError& err) {
    LOG(ERROR) << "Tokenize error: " << err.what();
  }
}

template< template<class, class, class, class> class M, class I>
template< class T>
void Config<M, I>::SetParam(const std::string& key, const T& value, bool is_string) {
  std::ostringstream oss;
  oss << value;
  Insert(key, oss.str(), is_string);
}

template< template<class, class, class, class> class M, class I>
void Config<M, I>::Insert(const std::string& key, const std::string& value, bool is_string) {
  I::Insert(config_map_, make_pair(key, value));
  I::Insert(is_string_map_, make_pair(key, is_string));
}

template< template<class, class, class, class> class M, class I>
const std::string& Config<M, I>::GetParam(const std::string& key) const {
  CHECK_NE(config_map_.find(key), config_map_.end()) << "key \"" << key << "\" not found in configure";
  return config_map_.find(key)->second;
}

template< template<class, class, class, class> class M, class I>
std::string Config<M, I>::ToProtoString(void) const {
  using namespace details;
  std::ostringstream oss;
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
