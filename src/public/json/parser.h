#pragma once

#include "json.h"

#include <iostream>

namespace zel {
namespace json {

class Parser {

  public:
    Parser();
    ~Parser();

    bool loadString(const std::string &str);

    Json parse();

  private:
    Json        parseNull();
    Json        parseBool();
    Json        parseNumber();
    std::string parseString();
    Json        parseArray();
    Json        parseObject();

    void skipWhiteSpace();
    bool inRange(long x, long lower, long upper) { return (x >= lower && x <= upper); }
    char getNextChar();

  private:
    std::string str_;   // json buffer
    int         index_; // 下标
};

} // namespace json
} // namespace zel