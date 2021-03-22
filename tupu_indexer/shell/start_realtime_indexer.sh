#!/bin/bash

./tupu_indexer  --server_name=realtime_indexer \
                --logbufsecs=0 \
                --log_dir=./ \
                --kafka_topic=tupu_realtime_indexer \
                --kafka_shards=4 \
                --es_url=http://es.cluster.sogou:9222/sjht-tupu-entity/_doc/ \
                --hb_redis_url=redis://:v0H6vxgLUHz7@sch.tupu.ms.redis.sogou:2631\
                --js_redis_url=redis://:v0H6vxgLUHz7@js.tupu.ms.redis.sogou:2778\
                --gd_redis_url=redis://:v0H6vxgLUHz7@gd.tupu.ms.redis.sogou:2759&