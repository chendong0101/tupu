/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _TRIAD_
#define _TRIAD_

#include <string>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/document/view.hpp>
#include <glog/logging.h>

using namespace std;
using bsoncxx::types::b_utf8;

struct Triad {
    string  _id;
    string  entity_id;
    string  kgc_id;
    string  predicate;
    string  value;
    string  to_entity_id;
    string  to_kgc_id;
    string  source;
    long    priority;
    string  op;
    long    op_time;

    private:
        string to_string(const b_utf8 &utf8) {
            return string(utf8.value.begin(), utf8.value.end());
        }
    public:
        void parseFromMongo(const bsoncxx::document::view& view) {
            _id = view["_id"].get_oid().value.to_string();
            entity_id = to_string(view["entity_id"].get_utf8());
            kgc_id = to_string(view["kgc_id"].get_utf8());
            predicate = to_string(view["predicate"].get_utf8());
            value = to_string(view["value"].get_utf8());
            to_entity_id = to_string(view["to_entity_id"].get_utf8());
            to_kgc_id = to_string(view["to_kgc_id"].get_utf8());
            source = to_string(view["source"].get_utf8());
            op = to_string(view["op"].get_utf8());
            op_time = view["op_time"].get_date().to_int64();
            priority = 0l;
            try {
                priority = view["priority"].get_int64().value;
            } catch (bsoncxx::exception e) {
                try {
                    priority = view["priority"].get_int32().value;
                } catch(bsoncxx::exception e) {
                    LOG(ERROR) << "exception " << _id;
                }
            }
        }
};

#endif
