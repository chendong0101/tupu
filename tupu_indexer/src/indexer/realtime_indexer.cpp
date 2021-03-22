/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "realtime_indexer.h"

#include <iostream>
#include <librdkafka/rdkafkacpp.h>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "thirdparty/json/nlohmann/json.hpp"
#include "utils/string_utils.h"
#include "clientproxy/redis_client.h"
#include "clientproxy/es_client.h"

using namespace std;
using namespace nlohmann;

DECLARE_string(kafka_topic);
DECLARE_string(kafka_brokers);
DECLARE_int32(redis_config_db);

void RealtimeIndexer::init() {
    ostringstream ss;
    ss << FLAGS_kafka_topic << ":partition_" << m_partition << ":offset";
    m_offsetKey = ss.str();
    bool ready = false;
    bool error = false;
    redis::getDoc(m_offsetKey, "hb", FLAGS_redis_config_db, [&](WFRedisTask* task) {
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
            this->m_startOffset = stol(val.string_value());
            ready = true;
        } else {
            LOG(ERROR) << "Get Offset Failed, with wrong type";
            this->m_startOffset = 0;
            ready = true;
        }
    });
    while (!ready && !error)
    {
        usleep(1000);
    }
    if (error) {
        exit(1);
    }
}

void RealtimeIndexer::startKafkaListener() {
    string errstr;
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    conf->set("metadata.broker.list", FLAGS_kafka_brokers, errstr);

    RdKafka::Consumer *consumer = RdKafka::Consumer::create(conf, errstr);
    if (!consumer) {
        LOG(ERROR) << "Failed to create producer: " << errstr ;
        exit(1);
    }
    LOG(INFO) << "% Created consumer " << consumer->name() << "\t offset: " << m_startOffset;

    RdKafka::Topic *topic = RdKafka::Topic::create(consumer, FLAGS_kafka_topic, tconf, errstr);
    if (!topic) {
        LOG(ERROR) << "Failed to create topic: " << errstr ;
        exit(1);
    }
    RdKafka::ErrorCode resp = consumer->start(topic, m_partition, m_startOffset);
    if (resp != RdKafka::ERR_NO_ERROR) {
        LOG(ERROR) << "Failed to start consumer: " << RdKafka::err2str(resp) ;
        exit(1);
    }
    while (true) {
        consumer->consume_callback(topic, m_partition, 1000,
                                   this, NULL);
        consumer->poll(1000);
    }
}

void RealtimeIndexer::consume_cb(RdKafka::Message &msg, void *opaque) {
    switch (msg.err())
    {
    case RdKafka::ERR__TIMED_OUT:
        break;

    case RdKafka::ERR_NO_ERROR:
    {
        char str[200];
        sprintf(str, "%.*s",
               static_cast<int>(msg.len()),
               static_cast<const char *>(msg.payload()));
        LOG(INFO) << str;
        json task = json::parse(str);
        string entityId = task["kgc_id"];
        if (task["operation"] == "upsert")
        {
            m_entityDispatcher->submit(entityId);
        }
        else if (task["operation"] == "delete")
        {
            LOG(INFO) << "delete entity: " << entityId;
            es::index_delete(entityId);
            redis::deleteDoc(entityId, "hb");
            redis::deleteDoc(entityId, "js");
            redis::deleteDoc(entityId, "gd");
        }

        redis::putDoc(m_offsetKey, to_string(msg.offset()), "hb", FLAGS_redis_config_db);
        break;
    }
    case RdKafka::ERR__PARTITION_EOF:
    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
    default:
        LOG(INFO) << "Consume failed: " << msg.errstr();
    }
}
