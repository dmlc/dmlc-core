#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <unordered_map>
#include <string>
#include <vector>
#include <assert.h>

namespace dmlc {


class Json;

class JsonValue {
public:
    static const JsonValue& nullValue() {
        static JsonValue value;
        return value;
    }

    JsonValue() {
        type_ = Null;
    }
    JsonValue(bool _bool) {
        type_ = Bool;
        data_.bool_ = _bool;
    }
    JsonValue(int _double) {
        type_ = Double;
        data_.double_ = _double;
    }
    JsonValue(double _double) {
        type_ = Double;
        data_.double_ = _double;
    }
    JsonValue(const Json& _json);
    JsonValue(const std::string& _string) {
        type_ = String;
        data_.string_ = new std::string(_string);
    }
    JsonValue(const char* _string) {
        type_ = String;
        data_.string_ = new std::string(_string);
    }
    JsonValue(const std::vector<JsonValue>& _array) {
        type_ = Array;
        data_.array_ = new std::vector<JsonValue>(_array);
    }

    JsonValue& operator=(const JsonValue& another) {
        //TODO: self assignment
        JsonValue tmp(another);
        swap(tmp);
        return *this;
    }

    //TODO: operator=
    JsonValue(const JsonValue& another);

    ~JsonValue();

    enum Type {Array, Bool, Double, Null, String, JsonObj};

    inline bool IsArray() const {return type_ == Array;}
    inline bool IsBool() const {return type_ == Bool;}
    inline bool IsDouble() const {return type_ == Double;}
    inline bool IsNull() const {return type_ == Null;}
    inline bool IsJson() const {return type_ == JsonObj;}
    inline bool IsString() const {return type_ == String;}

    inline Type type() const {return type_;}

    inline const std::vector<JsonValue>& ToArray() const {
        assert(IsArray());
        return *data_.array_;
    }
    inline bool ToBool() const {
        assert(IsBool());
        return data_.bool_;
    }
    inline double ToDouble() const {
        assert(IsDouble());
        return data_.double_;
    }
    inline const Json& ToJson() const {
        assert(IsJson());
        return *data_.json_;
    }
    inline const std::string& ToString() const {
        assert(IsString());
        return *data_.string_;
    }
private:
    union Data {
        std::string* string_;
        double double_;
        std::vector<JsonValue>* array_;
        Json* json_;
        bool bool_;
    } data_;

    Type type_;

    inline void swap(JsonValue& another) {
        Type ttype  = another.type_;
        another.type_ = type_;
        type_ = ttype;
        Data tdata = another.data_;
        another.data_ = data_;
        data_ = tdata;
    }
};

class Json {
public:
    Json() {}

    void SetValue(const std::string& key, const JsonValue& value);

    const JsonValue& GetValue(const std::string& key) const;

    //TODO: operator[] == !=

    std::string ToString() const;

    //TODO: error message
    static Json FromString(const std::string& json, std::string& err);
private:
    void appendValueToOutput(std::string& out, const JsonValue& value) const;

    std::unordered_map<std::string, JsonValue> map_;
};



} //namespace dmlc
#endif // include guard __JSON_HPP__
