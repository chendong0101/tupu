#!/bin/bash

./tupu_indexer  --server_name=full_indexer \
                --log_dir=./ \
                --es_url=http://es.cluster.sogou:9222/sjht-tupu-entity/_doc/ \
                --hb_redis_url=redis://:v0H6vxgLUHz7@sch.tupu.ms.redis.sogou:2631\
                --js_redis_url=redis://:v0H6vxgLUHz7@js.tupu.ms.redis.sogou:2778\
                --gd_redis_url=redis://:v0H6vxgLUHz7@gd.tupu.ms.redis.sogou:2759&