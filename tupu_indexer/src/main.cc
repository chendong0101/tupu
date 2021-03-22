/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include <glog/logging.h>
#include <gflags/gflags.h>
#include "workflow/WFFacilities.h"

#include "utils/property_utils.h"
#include "oplog/oplog_main.h"
#include "update_api/http_service.h"
#include "indexer/indexer_main.h"
#include "utils/segment_utils.h"
#include "config.h"

using namespace std;

static WFFacilities::WaitGroup wait_group(10);

void usage() {
    fprintf(stderr,
            "Usage: --server_name=<serivce> --config_file=<config> --log_dir=<log dir>\n"
            "Service: oplog_server | realtime_indexer | full_indexer \n"
            "example:"
            "./tupu_indexer  --server_name=realtime_indexer --config-file=../config/tupu_indexer.test.yml --log_dir=./"
    );
    exit(1);
}

int main(int argc, char *argv[]) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (FLAGS_server_name.empty()) {
        usage();
    }
    // 不支持 like 操作，暂不分词
    // utils::SegmentUtils::getInstance()->init(FLAGS_DICT_PATH, FLAGS_HMM_PATH, FLAGS_USER_DICT_PATH, FLAGS_IDF_PATH, FLAGS_STOP_WORD_PATH);

    if (FLAGS_server_name == "init_property_id") {
        utils::PropertyUtils::uploadProperties("/Users/chendong/work/cd_tupu/data/all_property.utf8");
    } else if (FLAGS_server_name == "oplog_server") {
        startOplogServer();
    } else if (FLAGS_server_name == "realtime_indexer") {
        startRealtimeIndexer();
    } else if (FLAGS_server_name == "full_indexer") {
        startFullIndexer();
    } else {
        usage();
    }
    // wait_group.wait();
    return 0;
}
