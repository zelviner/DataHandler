#pragma once

#include <string>
#include <vector>

namespace zel {

namespace utility {
class String {

  public:
    String();
    ~String();

    static std::wstring string2wstring(const std::string &wstr);
    static std::string  wstring2string(const std::wstring &str);

    static bool isUtf8(const std::string &str);

    /// @brief 将字符串转为 utf-8
    static std::string toUtf8(const std::string &str);

    static std::vector<std::string> split(const std::string &str, const std::string &delim);
    static int         stringReplace(std::string &sSrc, const std::string &sBefore, const std::string &sAfter);
    static void        stringToUpper(std::string &str);
    static void        stringToLower(std::string &str);
    static std::string stringFormat(const char *fmt, ...);
    static std::string stringSwap(const std::string buf);
    static std::string stringTrimLeft(std::string &str, const std::string &drop = " ");
    static std::string stringTrimRight(std::string &str, const std::string &drop = " ");
    static std::string stringTrim(std::string &str, const std::string &drop = " ");
    static std::string getDateTime();
    static bool        containsWith(const std::string &str, const std::string &sub);
    static bool        startWith(const std::string &str, const std::string &prefix);
    static bool        endWith(const std::string &str, const std::string &tail);
    static bool        isIpAddr(std::string ipstr);
    static bool        isDigit(const std::string str);
    static void        strSwap(const char *src, char *dst);

    static int asc2hex(const char *src, unsigned char *dst, int len);
    static int hex2asc(const unsigned char *src, char *dst, int len);

    static int  hex2bin(const char *src, unsigned char *dst, int len);
    static int  bin2hex(const unsigned char *src, char *dst, int len);
    static int  BcdToAscii(unsigned char *sSrc, unsigned char *sDest, int iLen);
    static int  AsciiToBcd(unsigned char *sSrc, unsigned char *sDest, int iLen);
    static void deleteIllegalChar(const char *src, char *dst);
    static bool isHexString(const char *src);
    static void char2Hex(unsigned char ch, char *szHex);
    static void hex2Char(char *szHex, unsigned char *rch);
    static void hex2CharStr(char *pszHexStr, char *pucCharStr, int iSize);
    static void char2HexStr(char *pucCharStr, char *pszHexStr, int iSize);
    static void fillChar(char *fix, char *dst, int len);
    static int  encodeBase64(char *pASCSrc, char *pBase64Res);
    static int  decodeBase64(char *pBase64Src, char *pASCRes);
    static void encryptString(char *src, char *dest);
    static void decryptString(char *src, char *dest);

  private:
};

} // namespace utility
} // namespace zel