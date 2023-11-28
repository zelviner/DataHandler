#include "string.h"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <regex>
#include <sstream>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace zel {
namespace utility {

String::String() {}
String::~String() {}

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

std::string String::toUtf8(const std::string &gbkData) { return wstring2string(string2wstring(gbkData)); }

std::vector<std::string> String::split(const std::string &str, const std::string &delim) {
    std::vector<std::string> ret;
    std::string::size_type   pos1, pos2;
    pos2 = str.find(delim);
    pos1 = 0;
    while (std::string::npos != pos2) {
        ret.push_back(str.substr(pos1, pos2 - pos1));

        pos1 = pos2 + delim.size();
        pos2 = str.find(delim, pos1);
    }
    if (pos1 != str.length()) ret.push_back(str.substr(pos1));

    return ret;
}

int String::stringReplace(std::string &sSrc, const std::string &sBefore, const std::string &sAfter) {
    int                    n    = 0;
    std::string::size_type spos = 0;
    std::string::size_type pos  = std::string::npos;

    if (sBefore.length() == 0) {
        return n;
    }

    while (spos < sSrc.size() && (pos = sSrc.find(sBefore, spos)) != std::string::npos) {
        sSrc.replace(pos, sBefore.size(), sAfter);
        n++;
        spos = pos + sAfter.size();
    }
    return n;
}

void String::stringToUpper(std::string &str) { std::transform(str.begin(), str.end(), str.begin(), ::toupper); }

void String::stringToLower(std::string &str) { std::transform(str.begin(), str.end(), str.begin(), ::tolower); }

std::string String::stringFormat(const char *fmt, ...) {
    std::string str;
    va_list     args;
    va_start(args, fmt);
    {
        int nLength = _vscprintf(fmt, args);
        nLength += 1; // 上面返回的长度是包含\0，这里加上
        std::vector<char> vectorChars(nLength);
        _vsnprintf(vectorChars.data(), nLength, fmt, args);
        str.assign(vectorChars.data());
    }
    va_end(args);

    return str;
}

std::string String::stringSwap(const std::string buf) {
    std::string::size_type len = buf.length();
    if (len < 2) // 不足2个字符，不交换
        return buf;

    char *pData = new char[len + 1];
    memset(pData, 0x00, len + 1);
    strncpy(pData, buf.c_str(), len);
    // char* pData = (char*)buf.c_str();
    for (std::size_t i = 0; i < len - 1; i += 2) {
        char chTemp      = *(pData + i);
        *(pData + i)     = *(pData + i + 1);
        *(pData + i + 1) = chTemp;
    }
    return pData;
}

std::string String::stringTrimLeft(std::string &str, const std::string &drop) {
    return str.erase(0, str.find_first_not_of(drop));
}

std::string String::stringTrimRight(std::string &str, const std::string &drop) {
    return str.erase(str.find_last_not_of(drop) + 1);
}

std::string String::stringTrim(std::string &str, const std::string &drop) {
    str.erase(str.find_last_not_of(drop) + 1);
    return str.erase(0, str.find_first_not_of(drop));
}

std::string String::getDateTime() {
    time_t timep;
    time(&timep);
    char tmp[255];
    memset(tmp, 0x00, sizeof(tmp));
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    return tmp;
}

bool String::containsWith(const std::string &str, const std::string &sub) {
    std::size_t idx = str.find(sub);
    if (idx != str.npos) {
        return true;
    } else {
        return false;
    }
}

bool String::startWith(const std::string &str, const std::string &prefix) {
    // 	return str.compare(0, prefix.size(), prefix) == 0;
    if (strncmp(str.c_str(), prefix.c_str(), prefix.size()) == 0) {
        return true;
    }
    return false;
}

bool String::endWith(const std::string &str, const std::string &tail) {
    if (str.compare(str.size() - tail.size(), tail.size(), tail) == 0) {
        return true;
    }
    return false;
}

void String::strSwap(const char *src, char *dst) {
    std::size_t len = strlen(src);

    char *pSrc = new char[len + 1];
    memset(pSrc, 0x00, len);
    strncpy(pSrc, src, len);

    if (len < 2) // 不足2个字符，不交换
    {
        memcpy(dst, pSrc, len);
        return;
    }

    for (std::size_t i = 0; i < len - 1; i += 2) {
        char Temp       = *(pSrc + i);
        *(pSrc + i)     = *(pSrc + i + 1);
        *(pSrc + i + 1) = Temp;
    }
    memcpy(dst, pSrc, len);
    delete[] pSrc;
    return;
}

int String::asc2hex(const char *src, unsigned char *dst, int len) {
    int i;
    for (i = 0; i < len / 2; i++) {
        sscanf(src + i * 2, "%02X", &dst[i]);
    }
    dst[i] = '\0';
    return i;
}

int String::hex2asc(const unsigned char *src, char *dst, int len) {
    int i;
    for (i = 0; i < len; i++) {
        sprintf(dst + i * 2, "%02X", src[i]);
    }
    dst[i * 2] = '\0';
    return i * 2;
}

int String::hex2bin(const char *src, unsigned char *dst, int len) {
    const char bin[256] = {
        /*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f */
        /* 00 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 10 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 20 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 30 */ 0, 1,  2,  3,  4,  5,  6,  7, 8, 9, 0, 0, 0, 0, 0, 0,
        /* 40 */ 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 50 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 60 */ 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 70 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 80 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 90 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* a0 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* b0 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* c0 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* d0 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* e0 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* f0 */ 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    int i;
    if (len == 0) len = strlen(src);
    for (i = 0; i < len; i += 2) {
        unsigned char h = src[i];
        unsigned char l = src[i + 1];
        dst[i / 2]      = bin[h] << 4 | bin[l];
    }
    dst[i / 2] = '\0';
    return i / 2;
}

int String::bin2hex(const unsigned char *src, char *dst, int len) {
    const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    int i;
    for (i = 0; i < len; i++) {
        unsigned char c = src[i];
        dst[i * 2]      = hex[c >> 4];
        dst[i * 2 + 1]  = hex[c & 0xf];
    }
    dst[i * 2] = '\0';
    return i * 2;
}

int String::BcdToAscii(unsigned char *sSrc, unsigned char *sDest, int iLen) {
    int           i, j;
    unsigned char chTmp1, chTmp2;
    unsigned char szTmp[1024 * 1000];
    j = 0;

    for (i = 0; i < iLen; i++) {
        chTmp1 = sSrc[i];

        chTmp2 = chTmp1 >> 4;
        if ((chTmp2 > 0x09) && (chTmp2 < 0x10)) {
            chTmp2 = chTmp2 + 0x07;
        }
        chTmp2   = chTmp2 + 0x30;
        szTmp[j] = chTmp2;

        j++;

        chTmp1 = sSrc[i];
        chTmp2 = chTmp1 & 0x0f;
        if ((chTmp2 > 0x09) && (chTmp2 < 0x10)) {
            chTmp2 = chTmp2 + 0x07;
        }
        chTmp2   = chTmp2 + 0x30;
        szTmp[j] = chTmp2;

        j++;
    }
    memcpy(sDest, szTmp, iLen * 2);
    return 0;
}

int String::AsciiToBcd(unsigned char *sSrc, unsigned char *sDest, int iLen) {
    unsigned char btTmp1, btTmp2, btLastByte, btResult[1024 * 1000];
    int           iCpt;

    btLastByte = iLen % 2;

    int nLen = iLen / 2;
    memset(btResult, 0, nLen + 1);
    for (iCpt = 0; iCpt < nLen; iCpt++) {
        btTmp1 = sSrc[2 * iCpt];
        btTmp2 = sSrc[2 * iCpt + 1];
        if ((btTmp1 >= 'A') && (btTmp1 <= 'F')) {
            btTmp1 -= 7;
        } else {
            if ((btTmp1 >= 'a') && (btTmp1 <= 'f')) {
                btTmp1 -= 39;
            }
        }
        if ((btTmp2 >= 'A') && (btTmp2 <= 'F')) {
            btTmp2 -= 7;
        } else {
            if ((btTmp2 >= 'a') && (btTmp2 <= 'f')) {
                btTmp2 -= 39;
            }
        }
        btResult[iCpt] = (unsigned char) ((btTmp1 << 4) | (btTmp2 & 0x0F));
    }
    if (btLastByte) {
        btTmp1 = sSrc[2 * iCpt];
        if ((btTmp1 >= 'A') && (btTmp1 <= 'F')) {
            btTmp1 -= 7;
        } else {
            if ((btTmp1 >= 'a') && (btTmp1 <= 'f')) {
                btTmp1 -= 39;
            }
        }
        btResult[nLen] = (unsigned char) (btTmp1 << 4);
    }
    if (btLastByte)
        memcpy(sDest, btResult, nLen + 1);
    else
        memcpy(sDest, btResult, nLen);
    return 0;
}

void String::deleteIllegalChar(const char *src, char *dst) {
    int len = strlen(src);

    int j = 0;
    for (int i = 0; i < len; i++) {
        unsigned char tmp = *(src + i);
        if (tmp < 0x21 || tmp > 0x7E) {
            continue;
        } else {
            strncpy(dst + j, src + i, 1);
            j++;
        }
    }
    dst[j] = '\0';

    return;
}

bool String::isHexString(const char *src) {
#define _ishex(_v1) (((_v1) >= '0' && (_v1) <= '9') || ((_v1) >= 'A' && (_v1) <= 'F') || ((_v1) >= 'a' && (_v1) <= 'f'))

    std::size_t len = strlen(src);
    for (std::size_t i = 0; i < len; i++) {
        if (!_ishex(src[i])) return false;
    }
    return true;
}

void String::char2Hex(unsigned char ch, char *szHex) {
    int           i;
    unsigned char byte[2];
    byte[0] = ch / 16;
    byte[1] = ch % 16;
    for (i = 0; i < 2; i++) {
        if (byte[i] >= 0 && byte[i] <= 9)
            szHex[i] = '0' + byte[i];
        else
            szHex[i] = 'a' + byte[i] - 10;
    }
    szHex[2] = 0;
}

void String::hex2Char(char *szHex, unsigned char *rch) {
    int i;
    for (i = 0; i < 2; i++) {
        if (*(szHex + i) >= '0' && *(szHex + i) <= '9')
            *rch = (*rch << 4) + (*(szHex + i) - '0');
        else if (*(szHex + i) >= 'a' && *(szHex + i) <= 'f')
            *rch = (*rch << 4) + (*(szHex + i) - 'a' + 10);
        else
            break;
    }
}

void String::hex2CharStr(char *pszHexStr, char *pucCharStr, int iSize) {
    int           i;
    unsigned char ch;
    if (iSize % 2 != 0) return;
    for (i = 0; i < iSize / 2; i++) {
        hex2Char(pszHexStr + 2 * i, &ch);
        pucCharStr[i] = ch;
    }
}

void String::char2HexStr(char *pucCharStr, char *pszHexStr, int iSize) {
    int  i;
    char szHex[3];
    pszHexStr[0] = 0;
    for (i = 0; i < iSize; i++) {
        char2Hex(pucCharStr[i], szHex);
        strcat(pszHexStr, szHex);
    }
}

void String::fillChar(char *fix, char *dst, int len) {
    // memset(dst, 0x00, strlen(dst));
    for (int i = 0; i < len; i++) {
        strcat(dst, fix);
    }
    dst[len] = '\0';
}

int String::encodeBase64(char *pASCSrc, char *pBase64Res) {
    int   srcLen;
    char  cMap[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *pCurSrc, *pCurRes;
    int   i, j, k, group;
    char  ch_Asc[4], ch_Base[4], ch, cha;

    if (!pASCSrc || !pBase64Res) return -1;

    srcLen = strlen(pASCSrc);
    if (srcLen < 1) {
        *pBase64Res = 0;
        return -1;
    }

    group   = srcLen / 3;
    i       = 0;
    pCurSrc = pASCSrc;
    pCurRes = pBase64Res;

    while (i < group) {
        for (j = 0; j < 3; j++) {
            if (pCurSrc[j] != 0) {
                ch_Asc[j] = *(pCurSrc + j);
            } else {
                for (k = j; k < 4; k++) {
                    ch_Asc[k] = 0;
                }
            }
        }
        ch_Base[0] = ((ch_Asc[0] >> 2) & 0x3f);
        *pCurRes   = cMap[ch_Base[0]];

        ch_Base[1] = ((ch_Asc[0] << 4) & 0x30);
        ch_Base[1] += ((ch_Asc[1] >> 4) & 0x0f);
        *(pCurRes + 1) = cMap[ch_Base[1]];

        ch_Base[2] = (ch_Asc[1] << 2) & 0x3c;
        ch_Base[2] += ((ch_Asc[2] >> 6) & 0x03);
        *(pCurRes + 2) = cMap[ch_Base[2]];

        ch_Base[3]     = (ch_Asc[2] & 0x3f);
        *(pCurRes + 3) = cMap[ch_Base[3]];

        i++;
        pCurSrc += 3;
        pCurRes += 4;
    }

    if ((srcLen % 3) == 1) {
        ch           = *pCurSrc;
        ch           = ch >> 2;
        ch           = ch & (0x3f);
        *(pCurRes++) = cMap[ch];

        ch           = *pCurSrc;
        ch           = ch << 4;
        ch           = ch & (0x30);
        *(pCurRes++) = cMap[ch];

        *(pCurRes++) = '=';
        *(pCurRes++) = '=';
        *pCurRes     = 0;
    } else if ((srcLen % 3) == 2) {
        ch           = *pCurSrc;
        ch           = (ch >> 2);
        ch           = ch & (0x3f);
        *(pCurRes++) = cMap[ch];

        ch  = *pCurSrc;
        cha = *(pCurSrc + 1);
        ch  = ch << 4;
        ch  = ch & (0x30);
        cha = (cha >> 4);
        cha = cha & (0x0f);
        cha += ch;
        *(pCurRes++) = cMap[cha];

        ch           = *(pCurSrc + 1);
        ch           = ch << 2;
        ch           = (ch & (0x3c));
        *(pCurRes++) = cMap[ch];
        *(pCurRes++) = '=';
        *pCurRes     = 0;
    } else
        *pCurRes = 0;
    return 1;
}

int String::decodeBase64(char *pBase64Src, char *pASCRes) {
    int srcLen;
    // char cMap[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *pCurSrc, *pCurRes;
    int   i = 0, j, group;
    char  ch[4], ch_Asc[4];

    srcLen = strlen(pBase64Src);
    if (srcLen % 4 != 0) {
        *pASCRes = 0;
        return -1;
    }

    group   = srcLen / 4;
    pCurSrc = pBase64Src;
    pCurRes = pASCRes;

    do {
        for (j = 0; j < 4; j++) {
            ch[j] = *(pCurSrc + j);

            if (ch[j] >= 'A' && ch[j] <= 'Z') {
                ch_Asc[j] = ch[j] - 'A';
            } else if (ch[j] >= 'a' && ch[j] <= 'z') {
                ch_Asc[j] = ch[j] - 'a' + 26;
            } else if (ch[j] >= '0' && ch[j] <= '9') {
                ch_Asc[j] = ch[j] - '0' + 52;
            } else if (ch[j] == '+') {
                ch_Asc[j] = 62;
            } else if (ch[j] == '/') {
                ch_Asc[j] = 63;
            } else if (ch[j] == '=') {
                ch_Asc[j] = '=';
            } else {
                *pASCRes = 0;
                return -1;
            }
        }
        *pCurRes = (ch_Asc[0] << 2) + (((ch_Asc[1] & 0x30) >> 4) & 0x0f);

        if (ch[2] != '=')
            *(pCurRes + 1) = ((ch_Asc[1] & 0x0f) << 4) + (((ch_Asc[2] & 0x3c) >> 2) & 0x3f);
        else {
            *(pCurRes + 1) = (ch_Asc[1] & 0x0f) << 4;
            *(pCurRes + 2) = 0;
            return 1;
        }
        if (ch[3] != '=')
            *(pCurRes + 2) = ((ch_Asc[2] & 0x03) << 6) + ch_Asc[3];
        else {
            *(pCurRes + 2) = (ch_Asc[2] & 0x03) << 6;
            *(pCurRes + 3) = 0;
            return 1;
        }
        i++;
        pCurRes += 3;
        pCurSrc += 4;

    } while (i < group);
    *(pCurRes) = 0;
    return 1;
}

void String::encryptString(char *src, char *dest) {
    char temp[4096];
    memset(temp, 0x00, sizeof(temp));
    encodeBase64(src, temp);
    char2HexStr(temp, dest, strlen(temp));
}

void String::decryptString(char *src, char *dest) {
    char temp[4096];
    memset(temp, 0x00, sizeof(temp));
    hex2CharStr(src, temp, strlen(src));
    decodeBase64(temp, dest);
}

} // namespace utility
} // namespace zel