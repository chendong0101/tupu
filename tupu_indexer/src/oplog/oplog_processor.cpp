/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "oplog_processor.h"

#include <unistd.h>
#include <functional>
#include <chrono>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <librdkafka/rdkafka.h>
#include <librdkafka/rdkafkacpp.h>
#include "clientproxy/redis_client.h"

using namespace std;
using namespace std::chrono;
using namespace std::placeholders;

DECLARE_int32(redis_triad_db);
DECLARE_string(kafka_brokers);

void OplogProcessor::init() {
    m_partition = RdKafka::Topic::PARTITION_UA;
    string errstr;
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
    conf->set("metadata.broker.list", FLAGS_kafka_brokers, errstr);

    m_producer = RdKafka::Producer::create(conf, errstr);
    if (!m_producer)
    {
        LOG(ERROR) << "Failed to create producer: " << errstr ;
        exit(1);
    }
    LOG(INFO) << "% Created producer " << m_producer->name() ;

    m_topic = RdKafka::Topic::create(m_producer, "tupu_realtime_indexer",
                                   tconf, errstr);
    if (!m_topic)
    {
        LOG(ERROR) << "Failed to create topic: " << errstr ;
        exit(1);
    }
}

void OplogProcessor::run()
{
    milliseconds ts = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch());

    while (true) {
        while (m_block) {
            usleep(100 * 1000);
        }

        milliseconds now_ts = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch());
        if (m_oplogBuffer->isFull() || now_ts - ts > milliseconds(1000 * 60l)) {
            ts = now_ts;
            m_block = true;
            m_oplogs = m_oplogBuffer->getBuffer();
            if (m_oplogs.empty()) {
                m_block = false;
                continue;
            }

            updateTriadIds();

            ProcessorCallback eraseStatusCallback = bind(&OplogProcessor::eraseStatus, this);

            ProcessorCallback sendMsg2KafkaCallback = bind(&OplogProcessor::sendMessage2Kafka, this, eraseStatusCallback);

            getEntityIds(sendMsg2KafkaCallback);
        } else {
            usleep(100 * 1000);
        }
    }
}

void OplogProcessor::updateTriadIds() {
    vector<string> insertIds;
    vector<string> deleteIds;

    for (Oplog log : m_oplogs) {
        if (log.op.compare("i") == 0 && !log.entity_id.empty()) {
            insertIds.push_back(log._id);
            insertIds.push_back(log.entity_id);
        }
        if (log.op.compare("d") == 0) {
            deleteIds.push_back(log._id);
        }
    }
    if (insertIds.size() > 0) {
        redis::putDocs(insertIds, "hb", FLAGS_redis_triad_db);
    }
    if (deleteIds.size() > 0) {
        redis::deleteDocs(deleteIds, "hb", FLAGS_redis_triad_db);
    }
}

void OplogProcessor::getEntityIds(ProcessorCallback callback) {
    vector<string> oids;

    for (Oplog log : m_oplogs) {
        if (log.entity_id.empty()) {
            oids.push_back(log._id);
        } else {
            m_entityIds.insert(log.entity_id);
        }
    }
    if (oids.size() > 0) {
        redis::getDocs(oids, "hb", FLAGS_redis_triad_db, bind(&OplogProcessor::getEntityIdsCallback, this, callback, _1));
    } else {
        callback();
    }
}

void OplogProcessor::getEntityIdsCallback(ProcessorCallback callback, WFRedisTask *task) {
    protocol::RedisResponse *resp = task->get_resp();
    int state = task->get_state();
    protocol::RedisValue result;

    if (state == WFT_STATE_SUCCESS) {
        resp->get_result(result);
        if (result.is_error())
        {
            LOG(ERROR) << "Error reply.";
            state = WFT_STATE_TASK_ERROR;
        }
    } else {
        LOG(ERROR) << "redis get Failed.";
        return;
    }

    if (result.is_array()) {
        for (int i = 0; i < result.arr_size(); i++) {
            protocol::RedisValue val = result.arr_at(i);
            if (val.is_nil())
            {
                continue;
            }
            if (val.is_string())
            {
                m_entityIds.insert(val.string_value());
            }
        }
    } else {
        LOG(ERROR) << "Error: Not a string value. " << result.string_value().c_str();
    }

    callback();
}

void OplogProcessor::sendMessage2Kafka(ProcessorCallback callback) {
    LOG(INFO) << "send Message to kafka: " << m_entityIds.size() ;
    for (auto entityId : m_entityIds) {
        RdKafka::ErrorCode resp =
            m_producer->produce(m_topic, m_partition,
                              RdKafka::Producer::RK_MSG_COPY,
                              const_cast<char *>(entityId.c_str()), entityId.size(),
                              NULL, NULL);
        if (resp != RdKafka::ERR_NO_ERROR)
            LOG(ERROR) << "% Produce failed: " << RdKafka::err2str(resp);
    }

    m_producer->poll(0);
    callback();
}

void OplogProcessor::eraseStatus() {
    LOG(INFO) ;
    m_oplogs.clear();
    m_entityIds.clear();
    m_block = false;
}
