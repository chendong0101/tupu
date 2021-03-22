/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _SEGMENT_UTILS_H
#define _SEGMENT_UTILS_H

#include "thirdparty/cppjieba/include/Jieba.hpp"

namespace utils {

    class SegmentUtils {
        private:
            cppjieba::Jieba *jieba;
        
        public:
            bool init(const string &dicpath, const string &hmmpath, const string &userdictpath, const string &idfpath, const string &stopwordpath) {
                jieba = new cppjieba::Jieba(dicpath, hmmpath, userdictpath, idfpath, stopwordpath);
                return true;
            }

            void cutAll(const string& sentence, vector<string>& words, bool include_original = false) const {
                if (include_original) {
                    words.push_back(sentence);
                }
                jieba->Cut(sentence, words);
            }

        private:
            SegmentUtils() = default;

        public:
            virtual ~SegmentUtils() {}
            static SegmentUtils *getInstance()
            {
                static SegmentUtils instance;
                return &instance;
            }
    };

};

#endif
