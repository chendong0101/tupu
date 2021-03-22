/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "oplog_listener.h"

#include <iterator>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include "utils/string_utils.h"
#include "clientproxy/redis_client.h"

using namespace std;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::closed_context;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

DECLARE_string(mongo_local_uri);
DECLARE_int32(redis_config_db);

void OplogListener::init() {
    mongocxx::instance instance{};
    bool ready = false;
    bool error = false;
    redis::getDoc(m_tsKey, "hb", FLAGS_redis_config_db, [&](WFRedisTask* task){
        protocol::RedisResponse *resp = task->get_resp();
        int state = task->get_state();
        protocol::RedisValue val;

        if (state == WFT_STATE_SUCCESS) {
            resp->get_result(val);
            if (val.is_error()) {
                state = WFT_STATE_TASK_ERROR;
            }
        } 
        if (state != WFT_STATE_SUCCESS) {
            LOG(ERROR) << "Get Offset Failed.";
            error = true;
            return;
        }
        if (val.is_string()) {
            vector<string> splits;
            utils::split(val.string_value(), splits, ":");
            if (splits.size() == 2) {
                this->m_ts.timestamp = stoi(splits[0]);
                this->m_ts.increment = stoi(splits[1]);
            }
            ready = true;
            return;
        }
        LOG(ERROR) << "Get Offset Failed, with wrong type";
        this->m_ts.timestamp = 0;
        this->m_ts.increment = 0;
        ready = true;
    });
    while (!ready && !error) {
        usleep(1000);
    }
    if (error) {
        exit(1);
    }
}
void OplogListener::run()
{
    mongocxx::uri uri(FLAGS_mongo_local_uri);
    mongocxx::client client(uri);
    mongocxx::database db = client["local"];
    mongocxx::collection coll = db["oplog.rs"];

    LOG(INFO) << m_ts.timestamp << ":" << m_ts.increment ;

    mongocxx::options::find opts{};
    opts.cursor_type(mongocxx::cursor::type::k_tailable);

    auto cursor =
        coll.find(document() << "ns" << "tupu.triad"
                             << "ts" << open_document << "$gt" << m_ts << close_document
                             << finalize,
                  opts);

    int i = 1;
    for (;;) {
        // LOG(INFO) << std::distance(cursor.begin(), cursor.end()) ;
        for (auto&& doc : cursor) {
            while (m_oplogBuffer->isFull())
            {
                usleep(1000 * 100);
            }

            bsoncxx::types::b_timestamp ts = doc["ts"].get_timestamp();
            Oplog oplog;
            oplog._id = doc["o"]["_id"].get_oid().value.to_string();
            oplog.op = string(doc["op"].get_utf8().value.begin(), doc["op"].get_utf8().value.end());
            oplog.timestamp = ts.timestamp;
            if (oplog.op.compare("i") == 0)
            {
                oplog.entity_id = string(doc["o"]["entity_id"].get_utf8().value.begin(), doc["o"]["entity_id"].get_utf8().value.end());
            }
            LOG(INFO) << oplog.toString() ;
            m_oplogBuffer->insert(oplog);
            if (i++ % 2000 == 0)
            {
                m_ts = ts;
                // bsoncxx 有 bug，我们只能将计就计了
                // string tss = to_string(ts.timestamp) + ":" + to_string(ts.increment);
                string tss = to_string(ts.increment) + ":" + to_string(ts.timestamp);
                redis::putDoc(m_tsKey, tss, "hb", FLAGS_redis_config_db);
                LOG(INFO) << tss ;
            }
        }
        LOG(INFO) << "tailing..." ;
        usleep(10 * 1000 * 1000); // sleep 10 seconds
    }
    LOG(INFO) << "sth wrong" ;
}
