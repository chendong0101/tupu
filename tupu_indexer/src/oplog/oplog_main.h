/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _OPLOG_MAIN_
#define _OPLOG_MAIN_

#include "oplog_buffer.h"
#include "oplog_processor.h"
#include "oplog_listener.h"

void startOplogServer() {
    OplogBuffer *buffer = new OplogBuffer(200);
    
    OplogListener listener(buffer);
    OplogProcessor processor(buffer);
}

#endif