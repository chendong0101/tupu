/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <glog/logging.h>
#include "clientproxy/redis_client.h"
#include "string_utils.h"
#include "property_utils.h"
using namespace utils;
using namespace std;

bool PropertyUtils::uploadProperties(const string &dicPath)
{
    ifstream ifs(dicPath.c_str());
    if (!ifs.is_open())
    {
        LOG(INFO) << "open " << dicPath << " failed" ;
        return false;
    }
    string line;

    vector<string> properties;
    for (size_t lineno = 1; getline(ifs, line); lineno++)
    {
        string line_utf8;
        vector<string> buf;
        split(line, buf, "\t");
        string key = buf[0];
        string value = buf[1];
        utils::trim(key);
        utils::trim(value);
        properties.push_back(key);
        properties.push_back(value);
        if (value == "769") {
            std::cout << key << "\t" << value << endl;
        }
        if (lineno % 1000 == 0) {
            redis::putDocs(std::move(properties), "js", 1);
            properties.clear();
            cout << to_string(lineno) << endl;
            usleep(10000);
        }
    }
    redis::putDocs(std::move(properties), "js", 1);
    std::cout << "finish" << std::endl;
    return true;
}
