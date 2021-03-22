/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef UTILS_PROPERTY_H
#define UTILS_PROPERTY_H

#include <string>
#include <map>
#include "consts.h"
#include "string_utils.h"
using namespace std;

namespace utils
{

    class PropertyUtils {

    public:
        PropertyUtils() = default;
        ~PropertyUtils() = default;

    public:
        static bool uploadProperties(const string &dicPath);
    };

}; // namespace utils

#endif