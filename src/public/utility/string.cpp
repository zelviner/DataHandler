#include "string.h"

#include <codecvt>

namespace zel {
namespace utility {

String::String() {}
String::~String() {}

// std::wstring String::string2wstring(const std::string &str) {
//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//     return converter.from_bytes(str);
// }
std::wstring String::string2wstring(const std::string &s) {
    std::string strLocale = setlocale(LC_ALL, "");
    const char *chSrc     = s.c_str();
    size_t      nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
    wchar_t    *wchDest   = new wchar_t[nDestSize];
    wmemset(wchDest, 0, nDestSize);
    mbstowcs(wchDest, chSrc, nDestSize);
    std::wstring wstrResult = wchDest;
    delete[] wchDest;
    setlocale(LC_ALL, strLocale.c_str());
    return wstrResult;
}

// std::string String::wstring2string(const std::wstring &wstr) {
//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//     return converter.to_bytes(wstr);
// }

std::string String::wstring2string(const std::wstring &ws) {
    std::string    strLocale = setlocale(LC_ALL, "");
    const wchar_t *wchSrc    = ws.c_str();
    size_t         nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
    char          *chDest    = new char[nDestSize];
    memset(chDest, 0, nDestSize);
    wcstombs(chDest, wchSrc, nDestSize);
    std::string strResult = chDest;
    delete[] chDest;
    setlocale(LC_ALL, strLocale.c_str());
    return strResult;
}

bool String::isUtf8(const std::string &str) {
    char          nBytes = 0; // UFT8可用1-6个字节编码,ASCII用一个字节
    unsigned char chr;
    bool          bAllAscii = true; // 如果全部都是ASCII, 说明不是UTF-8

    for (int i = 0; i < str.length(); i++) {
        chr = str[i];

        // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,
        // 但用一个字节存,最高位标记为0,o0xxxxxxx
        if ((chr & 0x80) != 0) bAllAscii = false;

        if (nBytes == 0) // 如果不是ASCII码,应该是多字节符,计算字节数
        {
            if (chr >= 0x80) {
                if (chr >= 0xFC && chr <= 0xFD)
                    nBytes = 6;
                else if (chr >= 0xF8)
                    nBytes = 5;
                else if (chr >= 0xF0)
                    nBytes = 4;
                else if (chr >= 0xE0)
                    nBytes = 3;
                else if (chr >= 0xC0)
                    nBytes = 2;
                else {
                    return false;
                }
                nBytes--;
            }
        } else // 多字节符的非首字节,应为 10xxxxxx
        {
            if ((chr & 0xC0) != 0x80) {
                return false;
            }
            nBytes--;
        }
    }

    if (nBytes > 0) // 违返规则
        return false;

    if (bAllAscii) // 如果全部都是ASCII, 说明不是UTF-8
        return false;

    return true;
}

std::string String::toUtf8(const std::string &gbkData) {
    return wstring2string(string2wstring(gbkData));
}

} // namespace utility

} // namespace common