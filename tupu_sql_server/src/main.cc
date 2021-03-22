/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include <thread>
#include <signal.h>

#include "config.h"
#include "grpc/grpc_service.h"
#include "http/http_service.h"
#include "service/property_id_service.h"
#include "utils/segment_utils.h"
#include "log/tupu_log.h"

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    //log init
    tupu::log::init(false);
    LOG_INFO("log init success\n");

    // 不支持 like 操作，暂不分词
    // utils::SegmentUtils::getInstance()->init(FLAGS_DICT_PATH, FLAGS_HMM_PATH, FLAGS_USER_DICT_PATH, FLAGS_IDF_PATH, FLAGS_STOP_WORD_PATH);
    // PropertyIdService::getInstance()->init();

    static HttpService httpService;
    std::thread httpServiceThread = std::thread(&HttpService::start, &httpService);

    static GRPCService grpcService;
    std::thread grpcServiceThread = std::thread(&GRPCService::start, &grpcService);

    signal(SIGINT, [](int signo) {
        LOG_ERROR("get signal to stop: %d", signo);
        grpcService.terminate();
        httpService.terminate();
    });
    httpServiceThread.join();
    grpcServiceThread.join();
    LOG_INFO("finished");
    return 0;
}