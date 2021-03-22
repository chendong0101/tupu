/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>
#include <algorithm> 
#include "opencc/opencc.h"

#include "type_define.h"

using namespace std;

// static const opencc::SimpleConverter converter("t2s.json");
static const opencc_t cc = opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);

namespace utils
{
    size_t findKeyWord(string str, string substr);

    void split(const string &src, vector<string> &res, const string &pattern, size_t maxsplit = string::npos);

    void char2Half(string &str);

    size_t gbk2utf8(const string &gbk_str, string &utf8_str);

    size_t utf82gbk(const string &utf8_str, string &gbk_str);

    string urlEncode(const string &str);

    string urlDecode(const string &input);

    termID32_t hash32(const string &src);
    termID32_t FNV_1a(const string &src);

    termID64_t hash64(const string &src);
    
    inline void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !(ch == ',' || std::isspace(ch));
                }));
    }

    inline void rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !(ch == ',' || std::isspace(ch));
                }).base(),
                s.end());
    }

    inline void trim(std::string &s)
    {
        ltrim(s);
        rtrim(s);
    }

    inline unsigned char toHex(unsigned char x)
    {
        return x > 9 ? x + 55 : x + 48;
    }

    inline int char2num(char ch)
    {
        if (ch >= '0' && ch <= '9')
            return (ch - '0');
        if (ch >= 'a' && ch <= 'f')
            return (ch - 'a' + 10);
        if (ch >= 'A' && ch <= 'F')
            return (ch - 'A' + 10);
        return -1;
    }

    inline int normalize(string &s) {
        trim(s);
        char2Half(s);
        for_each(s.begin(), s.end(), [](char &c) {
            if (isupper(c)) {
                c = tolower(c);
            }
        });
        // s = converter.Convert(s);
        char* simpleStr = opencc_convert_utf8(cc, s.c_str(), s.length());
        s = string(simpleStr);
        opencc_convert_utf8_free(simpleStr);
        return 0;
    }

    inline bool isUnsignedDigit(string s)
    {
        for (auto c : s)
        {
            if (!isdigit(c))
            {
                return false;
            }
        }
        return true;
    }

} // namespace utils

#endif //_STRING_UTILS_H 