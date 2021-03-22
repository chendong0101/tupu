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
yum install cmake3 workflow-devel gtest-devel opencc-devel glog-devel gflags-devel protobuf3-devel protobuf3-compiler libticonv-devel
cd tupu_sql_server
mkdir build
cd build
cmake ..
make
~~~

## 启动服务
二进制位置 ``./tupu_sql_server``
~~~sh
nohup ./tupu_sql_server --log_dir=./ \
                    --server_port=8888 \
                    --redis_url=redis://:v0H6vxgLUHz7@sch.tupu.ms.redis.sogou:2631 \
                    --DICT_PATH=/search/ssd/daemon/tupu_cd/data/dict/jieba.dict.utf8 \
                    --HMM_PATH=/search/ssd/daemon/tupu_cd/data/dict/hmm_model.utf8 \
                    --USER_DICT_PATH=/search/ssd/daemon/tupu_cd/data/dict/user.dict.utf8 \
                    --IDF_PATH=/search/ssd/daemon/tupu_cd/data/dict/idf.utf8 \
                    --STOP_WORD_PATH=/search/ssd/daemon/tupu_cd/data/dict/stop_words.utf8 &
~~~

# 配置文件
(DICT_PATH,        "/Users/chendong/work/cd_tupu/data/dict/jieba.dict.utf8",   "最大概率法(MPSegment: Max Probability)分词所使用的词典路径");
(HMM_PATH,         "/Users/chendong/work/cd_tupu/data/dict/hmm_model.utf8",    "隐式马尔科夫模型(HMMSegment: Hidden Markov Model)分词所使用的词典路径");
(USER_DICT_PATH,   "/Users/chendong/work/cd_tupu/data/dict/user.dict.utf8",    "用户自定义词典路径");
(IDF_PATH,         "/Users/chendong/work/cd_tupu/data/dict/idf.utf8",          "IDF路径");
(STOP_WORD_PATH,   "/Users/chendong/work/cd_tupu/data/dict/stop_words.utf8",   "停用词路径");


(server_port,       8888,                                                       "service port");
(redis_url,        "redis://:v0H6vxgLUHz7@sch.tupu.ms.redis.sogou:2631",       "redis url");

(redis_doc_db,                      0,                                          "db in redis: entity id ---> doc");
(redis_property_id_db,              1,                                          "db in redis: property ---> id");



