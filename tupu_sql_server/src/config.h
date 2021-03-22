/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <gflags/gflags.h>


DEFINE_string(DICT_PATH,        "/Users/chendong/work/cd_tupu/data/dict/jieba.dict.utf8",   "最大概率法(MPSegment: Max Probability)分词所使用的词典路径");
DEFINE_string(HMM_PATH,         "/Users/chendong/work/cd_tupu/data/dict/hmm_model.utf8",    "隐式马尔科夫模型(HMMSegment: Hidden Markov Model)分词所使用的词典路径");
DEFINE_string(USER_DICT_PATH,   "/Users/chendong/work/cd_tupu/data/dict/user.dict.utf8",    "用户自定义词典路径");
DEFINE_string(IDF_PATH,         "/Users/chendong/work/cd_tupu/data/dict/idf.utf8",          "IDF路径");
DEFINE_string(STOP_WORD_PATH,   "/Users/chendong/work/cd_tupu/data/dict/stop_words.utf8",   "停用词路径");


DEFINE_int32(server_port,       8888,                                                       "service port");
DEFINE_string(es_url,           "http://10.164.41.36:9200/ntriple-index-001/_search",       "elastic search url");
DEFINE_string(redis_url,        "redis://:v0H6vxgLUHz7@sch.tupu.ms.redis.sogou:2631",       "redis url");

DEFINE_int32(redis_doc_db,                      0,                                          "db in redis: entity id ---> doc");
DEFINE_int32(redis_property_id_db,              1,                                          "db in redis: property ---> id");

#endif