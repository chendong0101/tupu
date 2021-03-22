/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "string_utils.h"

#include <iostream>
#include <iconv.h>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <openssl/md5.h>
#include "type_define.h"

using namespace std;

namespace utils
{
    size_t findKeyWord(string str, string substr)
    {
        int pos = 0;
        int quotation = 0;
        int brackets = 0;
        while (pos < str.length())
        {
            if (str[pos] == '"')
            {
                quotation++;
                pos++;
                continue;
            }
            else if (quotation % 2 == 0 && str[pos] == '(')
            {
                brackets++;
                pos++;
                continue;
            }
            else if (quotation % 2 == 0 && str[pos] == ')')
            {
                brackets--;
                pos++;
                continue;
            }

            if (brackets == 0 && quotation % 2 == 0)
            {
                if (str.compare(pos, substr.size(), substr) == 0)
                {
                    return pos;
                }
            }
            pos++;
        }
        return -1;
    }
    void split(const string &src, vector<string> &res, const string &pattern, size_t maxsplit)
    {
        res.clear();
        size_t start = 0;
        size_t end = 0;
        string sub;
        while (start < src.size())
        {
            end = src.find_first_of(pattern, start);
            if (string::npos == end || res.size() >= maxsplit)
            {
                sub = src.substr(start);
                res.push_back(sub);
                return;
            }
            sub = src.substr(start, end - start);
            res.push_back(sub);
            start = end + 1;
        }
        return;
    }

    void char2Half(std::string &input)
    {
        std::string temp;
        for (size_t i = 0; i < input.size(); i++)
        {
            if (((input[i] & 0xF0) ^ 0xE0) == 0)
            {
                int old_char = (input[i] & 0xF) << 12 | ((input[i + 1] & 0x3F) << 6 | (input[i + 2] & 0x3F));
                if (old_char == 0x3000)
                { // blank
                    char new_char = 0x20;
                    temp += new_char;
                }
                else if (old_char >= 0xFF01 && old_char <= 0xFF5E)
                { // full char
                    char new_char = old_char - 0xFEE0;
                    temp += new_char;
                }
                else
                { // other 3 bytes char
                    temp += input[i];
                    temp += input[i + 1];
                    temp += input[i + 2];
                }
                i = i + 2;
            }
            else
            {
                temp += input[i];
            }
        }
        input = temp;
    }

    size_t gbk2utf8(const string &gbk_str, string &utf8_str)
    {
        iconv_t cd = iconv_open("utf-8", "GB18030");
        if (cd == (void *)-1L)
        {
            fprintf(stderr, "iconv_open fail, errno=%d\n", errno);
            return -1;
        }
        size_t inbytesleft = gbk_str.size();
        char *in = const_cast<char *>(gbk_str.c_str());

        char outbuff[10240];
        char *out = outbuff;
        size_t outbytesleft = sizeof(outbuff) - 1;

        size_t ret = iconv(cd, &in, &inbytesleft, &out, &outbytesleft);
        iconv_close(cd);
        *out = 0;
        if (ret == size_t(-1))
        {
            fprintf(stderr, "iconv failed: %s, value=%s\n", strerror(errno), gbk_str.c_str());
        }
        else
        {
            string s = outbuff;
            utf8_str.swap(s);
        }
        return ret;
    }

    size_t utf82gbk(const string &utf8_str, string &gbk_str)
    {
        iconv_t cd = iconv_open("GB18030", "utf-8");
        if (cd == (void *)-1L)
        {
            // todo
            fprintf(stderr, "iconv_open fail, errno=%d\n", errno);
            return -1;
        }
        size_t inbytesleft = utf8_str.size();
        char *in = const_cast<char *>(utf8_str.c_str());

        char outbuff[10240];
        char *out = outbuff;
        size_t outbytesleft = sizeof(outbuff) - 1;

        size_t ret = iconv(cd, &in, &inbytesleft, &out, &outbytesleft);
        iconv_close(cd);
        *out = 0;
        if (ret == size_t(-1))
        {
            fprintf(stderr, "iconv failed: %s, value=%s\n", strerror(errno), gbk_str.c_str());
        }
        else
        {
            string s = outbuff;
            gbk_str.swap(s);
        }
        return ret;
    }

    string urlEncode(const string &str)
    {
        string strTemp = "";
        size_t length = str.length();

        for (size_t i = 0; i < length; i++)
        {
            if ((unsigned char)str[i] <= 127 && (isalnum((unsigned char)str[i]) ||
                                                 (str[i] == '-') ||
                                                 (str[i] == '_') ||
                                                 (str[i] == '.') ||
                                                 (str[i] == '~')))
                strTemp += str[i];
            else if (str[i] == ' ')
                strTemp += "+";
            else
            {
                strTemp += '%';
                strTemp += toHex((unsigned char)str[i] >> 4);
                strTemp += toHex((unsigned char)str[i] % 16);
            }
        }
        return strTemp;
    }
    string urlDecode(const string &input)
    {
        string output = "";
        size_t i = 0;
        size_t len = input.size();
        int num1, num2;
        while (i < len)
        {
            char ch = input[i];
            switch (ch)
            {
            case '+':
                output.append(1, ' ');
                i++;
                break;
            case '%':
                if (i + 2 < len)
                {
                    num1 = char2num(input[i + 1]);
                    num2 = char2num(input[i + 2]);
                    if (num1 != -1 && num2 != -1)
                    {
                        char res = (char)((num1 << 4) | num2);
                        output.append(1, res);
                        i += 3;
                        break;
                    }
                }
            default:
                output.append(1, ch);
                i++;
                break;
            }
        }
        return output;
    }

    termID32_t hash32(const string &src)
    {
        MD5_CTX state;
        uint8_t buf[MD5_DIGEST_LENGTH];

        MD5_Init(&state);
        MD5_Update(&state, src.c_str(), src.length());
        MD5_Final(buf, &state);

        termID32_t digest;
        memcpy((uint8_t *)&digest, buf, 4);
        return digest;
    }

    // FNV_1a: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    // hash 算法对比：https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
    termID32_t FNV_1a(const string &src) 
    {
        uint32_t FNV_prime = 16777619;
        uint32_t FNV_offset_basis = 2166136261;

        termID32_t h = FNV_offset_basis;
        for_each(src.begin(), src.end(), [&](const char &c) {
            h = (h ^ c) * FNV_prime;
        });
        return h;
    }

    termID64_t hash64(const string &src)
    {
        termID64_t h = FNV_1a(src);
        termID32_t l = hash32(src);
        return (h << 32) | l;
    }

} // namespace utils