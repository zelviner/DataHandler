#pragma once

#include <string>

namespace zel {
namespace utility {

class String {

  public:
    String();
    ~String();

    static std::wstring string2wstring(const std::string &wstr);

    static std::string wstring2string(const std::wstring &str);

    static bool isUtf8(const std::string &str);

    /// @brief 将字符串转为 utf-8
    static std::string toUtf8(const std::string &str);

  private:
};

} // namespace utility

} // namespace common