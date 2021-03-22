/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <gflags/gflags.h>

DEFINE_string(DICT_PATH,        "/home/snswenwen/tupu_new/ddata/dict/jieba.dict.utf8",           "最大概率法(MPSegment: Max Probability)分词所使用的词典路径");
DEFINE_string(HMM_PATH,         "/home/snswenwen/tupu_new/ddata/dict/hmm_model.utf8",            "隐式马尔科夫模型(HMMSegment: Hidden Markov Model)分词所使用的词典路径");
DEFINE_string(USER_DICT_PATH,   "/home/snswenwen/tupu_new/ddata/dict/user.dict.utf8",            "用户自定义词典路径");
DEFINE_string(IDF_PATH,         "/home/snswenwen/tupu_new/data/dict/idf.utf8",                   "IDF路径");
DEFINE_string(STOP_WORD_PATH,   "/home/snswenwen/tupu_new/data/dict/stop_words.utf8",            "停用词路径");


DEFINE_string(server_name,      "",                                                                 "server name");

DEFINE_string(es_url,           "http://10.164.41.36:9200/ntriple-index-001/_doc/",                 "elastic search url");
DEFINE_string(hb_redis_uri,     "redis://127.0.0.1:6379",                                           "redis url");
DEFINE_string(js_redis_uri,     "redis://127.0.0.1:6379",                                           "redis url");
DEFINE_string(gd_redis_uri,     "redis://127.0.0.1:6379",                                           "redis url");

DEFINE_string(mongo_uri,        "mongodb://tupu:tupu_sogou%402019@mongo.tupu.hn.sogou:41728/tupu?authSource=tupu",       "mongodb for triad");
DEFINE_string(mongo_local_uri,  "mongodb://tupulocal:tupulocal_sogou%402019@10.145.49.53:40001",    "mongodb for oplog");

DEFINE_string(kafka_topic,      "tupu_realtime_indexer",                                            "kafka topic");
DEFINE_int32(kafka_shards,      4,                                                                  "kafka shards");
DEFINE_string(kafka_brokers,    "kafka002.wd.hb.ted:9092,kafka004.wd.hb.ted:9092,kafka019.wd.hb.ted:9092", "kafka brokers");


DEFINE_int32(redis_doc_db,              0,                                                          "db in redis: entity id ---> doc");
DEFINE_int32(redis_property_id_db,      1,                                                          "db in redis: property ---> id");
DEFINE_int32(redis_triad_db,            2,                                                          "db in redis: triad id ---> entity id");
DEFINE_int32(redis_config_db,           4,                                                          "db in redis: control configs, for example: oplog:tupu:ts, tupu_realtime_indexer:partition_0:offset etc.");

#endif