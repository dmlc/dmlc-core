#include <dmlc/json.h>
#include <dmlc/logging.h>
#include <iostream>
#include <vector>
#include <assert.h>
using namespace dmlc;
using namespace std;

int main() {
    Json j;
    j.SetValue("first", "James");
    vector<JsonValue> array = {"", 1, 2, 3, true};
    j.SetValue("array", array);
    j.SetValue("null", JsonValue());
    j.SetValue("double", 1.0);

    CHECK(j.GetValue("first").ToString() == "James");
    CHECK(j.GetValue("array").ToArray()[1].ToDouble() == 1.0);
    CHECK(j.GetValue("array").ToArray()[4].ToBool() == true);

    cout << "Input values: {\"double\":1.0,\"null\":null,\"array\":[\"\",1.000000,2.000000,3.000000,true],\"first\":\"James\"}" << endl;
    cout << "Json result: " << j.ToString() << endl;


    std::string err;
    std::string str = "{\"abc\": \"def\", \"arr\":[1, 2, 3, \"ele\", true], \"obj\":{\"this\":false}, \"obj2\":{\"this\":12.03}";
    Json parsed = Json::FromString(str, err);
    cout << endl;
    cout << "Testing json: " << str << endl;
    CHECK(parsed.GetValue("abc").ToString() == "def") << "test abc";
    CHECK(parsed.GetValue("arr").ToArray()[3].ToString() == "ele") << "test ele";
    CHECK(parsed.GetValue("obj").ToJson().ToString() == "{\"this\":false}") << "test obj.this";
    CHECK(parsed.GetValue("obj2").ToJson().GetValue("this").ToDouble() == 12.03) << "test obj2.this";
    CHECK(parsed.GetValue("doesn't exist").IsNull());
    return 0;
}
