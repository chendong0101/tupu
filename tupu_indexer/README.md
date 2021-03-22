# TUPU SQL SERVER
tupu_sql_server 是目前线上图谱 SQL 查询服务，整体替代之前的 kbcache。kbcache 基于大搜的架构改造而来，经过多年迭代，维护成本较高。tupu_sql_server 基于 workflow 框架 + ElasticSearch + Redis 重构。
<!-- MarkdownTOC -->

- [编译和部署](#%E7%BC%96%E8%AF%91%E5%92%8C%E9%83%A8%E7%BD%B2)
	- [准备工作](#%E5%87%86%E5%A4%87%E5%B7%A5%E4%BD%9C)
	- [本地编译](#%E6%9C%AC%E5%9C%B0%E7%BC%96%E8%AF%91)
	- [启动服务](#%E5%90%AF%E5%8A%A8%E6%9C%8D%E5%8A%A1)
- [配置文件](#%E9%85%8D%E7%BD%AE%E6%96%87%E4%BB%B6)

<!-- /MarkdownTOC -->

# 编译和部署

## 准备工作
建议centos6以上系统，1个编译好的C++二进制程序

## 本地编译
~~~sh
yum install cmake3 workflow-devel gtest-devel librdkafka-devel opencc-devel glog-devel gflags-devel protobuf3-devel protobuf3-compiler 
yum install rh-mongodb36-mongo-cxx-driver-devel rh-mongodb36-boost-driver-devel libticonv-devel
cd tupu_sql_server
mkdir build
cd build
cmake ..
make
~~~

## 启动服务
二进制位置 ``./tupu_sql_server``
~~~sh
nohup ./tupu_indexer  --server_name=realtime_indexer \
                --logbufsecs=0 \
                --log_dir=./ \
                --kafka_topic=tupu_realtime_indexer_urgency \
                --kafka_shards=2 \
                --es_url=http://es.cluster.sogou:9222/sjht-tupu-entity/_doc/ \
                --hb_redis_url=redis://:v0H6vxgLUHz7@sch.tupu.ms.redis.sogou:2631\
                --js_redis_url=redis://:v0H6vxgLUHz7@js.tupu.ms.redis.sogou:2778\
                --gd_redis_url=redis://:v0H6vxgLUHz7@gd.tupu.ms.redis.sogou:2759&
~~~

# 配置文件
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
DEFINE_int32(redis_config_db,           4,                                                          "db in redis: control configs, for example: oplog:tupu:ts, tupu_realtime_indexer:partition_0:offset etc.");