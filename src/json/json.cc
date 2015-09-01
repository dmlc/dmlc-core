#include "dmlc/json.h"
#include <utility>

namespace dmlc {

JsonValue::JsonValue(const Json& _json) {
    type_ = JsonObj;
    data_.json_ = new Json(_json);
}

JsonValue::JsonValue(const JsonValue& another) {
    type_ = another.type_;
    switch(type_) {
        case Null:
            break;
        case Double:
            data_.double_ = another.data_.double_;
            break;
        case String:
            data_.string_ = new std::string(*another.data_.string_);
            break;
        case JsonObj:
            data_.json_ = new Json(*another.data_.json_);
            break;
        case Bool:
            data_.bool_ = another.data_.bool_;
            break;
        case Array:
            data_.array_ = new std::vector<JsonValue>(*another.data_.array_);
            break;
    }
}


JsonValue::~JsonValue() {
    if (IsArray()) {
        delete data_.array_;
    } else if (IsString()) {
        delete data_.string_;
    } else if (IsJson()) {
        delete data_.json_;
    }
    type_ = Null;
}

class Parser {
public:
    Parser(const std::string& _json) :  parsed_(NULL), p_(0), json_(_json) {}
    ~Parser() { delete parsed_; }
    inline const Json& parse() {
        size_t tmp = 0;
        return parse(tmp);
    }
private:
    //NOTE: I assume we are parsing simple jsons so there wouldn't be a
    // concern of objects copy at this point
    inline const Json& parse(size_t& start) {
        p_ = start;
        delete parsed_;
        parsed_ = new Json();
        parseJson();
        start = p_;
        // if (p_ < json_.length()) warn!
        return *parsed_;
    }

    inline bool parseJson() {
        size_t mark = p_;
        if (!expect('{')) {
            return false;
        }

        while (true) {
            std::string key;
            if (!parseKey(key)) break;
            if (!expect(':')) {
                p_ = mark;
                return false;
            }
            JsonValue value;
            if (!parseValue(value)) {
                p_ = mark;
                return false;
            }
            parsed_->SetValue(key, value);
            if (!expect(',')) {
                break;
            }
        }

        if (!expect('}')) {
            p_ = mark;
            return false;
        }

        return true;
    }

    inline bool parseKey(std::string& key) {
        return parseString(key);
    }

    inline bool parseString(std::string& str) {
        size_t mark = p_;
        if (!expect('"')) return false;
        size_t begin = p_;
        size_t end = p_;
        while(end < json_.length()) {
            if (json_[end] == '"') break;

            if (json_[end] == '\\') {
                if (end + 1 >= json_.length()) {
                    p_ = mark;
                    return false;
                }
                //escaped. whatever.
                end++;
            }
            end++;
        }
        str = json_.substr(begin, end - begin);
        p_ = end + 1;
        return true;
    }

    inline bool parseValue(JsonValue& value) {
        readWhiteSpaces();
        if (p_ == json_.length()) return false;
        char ch = json_[p_];
        if (ch == 't') {
            if (!expect("true")) return false;
            value = JsonValue(true);
        } else if (ch == 'f') {
            if (!expect("false")) return false;
            value = JsonValue(false);
        } else if (ch == 'n') {
            if (!expect("null")) return false;
            value = (JsonValue());
        } else if (ch == '.' || ('0' <= ch && ch <= '9')) {
            double d;
            if (!parseDouble(d)) return false;
            value = JsonValue(d);
        } else if (ch == '"') {
            std::string str;
            if (!parseString(str)) return false;
            value = JsonValue(str);
        } else if (ch == '{') {
            Parser subParser(json_);
            const Json& subJson = subParser.parse(p_);
            value = JsonValue(subJson);
        } else if (ch == '[') {
            std::vector<JsonValue> array;
            if (!parseArray(array)) return false;
            value = JsonValue(array);
        } else {
            return false;
        }
        return true;
    }

    //TODO: restore p_
    inline bool parseArray(std::vector<JsonValue>& array) {
        size_t mark = p_;
        if (!expect('[')) return false;

        while (true) {
            JsonValue value;
            if (!parseValue(value)) break;
            array.push_back(value);
            if (!expect(',')) {
                break;
            }
        }

        if (!expect(']')) {
            p_ = mark;
            return false;
        }
        return true;
    }

    inline bool parseDouble(double& d) {
        static std::string legalChars = ".0123456789e";
        size_t start = p_;
        size_t end = p_;
        while (end < json_.length() && legalChars.find(json_[end]) != std::string::npos) {
            end++;
        }
        if (end == json_.length()) {
            return false;
        }
        size_t sz;
        double result = std::stod(json_.substr(start, end - start), &sz);
        if (start + sz == end) {
            d = result;
            p_ = end;
            return true;
        }
        return false;
    }

    inline bool expect(const std::string& str) {
        size_t cur = p_;
        size_t i = 0;
        while (cur < json_.length() && i < str.length() && json_[cur++] == str[i++]);
        if (i == str.length()) {
            p_ = cur;
            return true;
        }
        return false;
    }

    inline bool expect(char ch) {
        readWhiteSpaces();
        if (p_ == json_.length() || json_[p_] != ch) return false;
        p_++;
        return true;
    }

    inline void readWhiteSpaces() {
        static const std::string white = " \t\n\r";
        while(p_ < json_.length() && white.find(json_[p_]) != std::string::npos) {
            p_++;
        }
    }

    Json* parsed_;
    size_t p_;
    const std::string& json_;
};


//TODO: error message
Json Json::FromString(const std::string& json, std::string& err) {
    Parser parser(json);
    return parser.parse();
}

const JsonValue& Json::GetValue(const std::string& key) const {
    auto iter = map_.find(key);
    if (iter != map_.end()) {
        return iter->second;
    }
    return JsonValue::nullValue();
}

std::string Json::ToString() const {
    std::string ret("{");
    for (const std::pair<std::string, JsonValue>& p : map_) {
        ret += "\"" + p.first + "\":";
        appendValueToOutput(ret, p.second);
    }
    if (ret.back() == ',') ret.pop_back();
    ret += "}";
    return ret;
}

void Json::SetValue(const std::string& key, const JsonValue& value) {
    map_[key] = value;
}

void Json::appendValueToOutput(std::string& out, const JsonValue& value) const {
    switch (value.type()) {
        case JsonValue::Null:
            out += "null";
            break;
        case JsonValue::Double:
            out += std::to_string(value.ToDouble());
            break;
        case JsonValue::String:
            out += "\"" + value.ToString() + "\"";
            break;
        case JsonValue::JsonObj:
            out += value.ToJson().ToString();
            break;
        case JsonValue::Bool:
            out += value.ToBool() ? "true" : "false";
            break;
        case JsonValue::Array:
            auto array = value.ToArray();
            out += '[';
            for (size_t i = 0; i < array.size(); ++i) {
                appendValueToOutput(out, array[i]);
            }
            if (out.back() == ',') out.pop_back();
            out += ']';
            break;
    }
    out += ',';
}

} //namespace dmlc
