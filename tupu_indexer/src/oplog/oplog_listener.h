/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _ES_OPLOG_LISTENER_H_
#define _ES_OPLOG_LISTENER_H_

#include <thread>
#include <string>
#include <bsoncxx/types.hpp>
#include "oplog_buffer.h"

class OplogListener {
    private:
        OplogBuffer *m_oplogBuffer;
        std::thread m_thread;
        std::string m_tsKey;
        bsoncxx::types::b_timestamp m_ts;

    private:
        void init();
        void run();
    public: 
        OplogListener(OplogBuffer *buf) : m_oplogBuffer(buf) {
            m_tsKey = "oplog:tupu:ts";
            init();
            m_thread = std::thread(&OplogListener::run, this);
        }
        ~OplogListener() {
            m_thread.join();
        }
};

#endif