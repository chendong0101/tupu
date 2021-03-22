/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "rw_lock.h"

namespace utils {
namespace lock {

void rw_lock::lockRead() {
    std::unique_lock<std::mutex> gurad(m_Lock);
    m_readCond.wait(gurad, [=] { return 0 == m_writeCount; });
    ++m_readCount;
}

void rw_lock::unlockRead() {
    std::unique_lock<std::mutex> gurad(m_Lock);
    if (0 == (--m_readCount)
        && m_writeCount > 0) {
        m_writeCond.notify_one();
    }
}

void rw_lock::lockWrite() {
    std::unique_lock<std::mutex> gurad(m_Lock);
    ++m_writeCount;
    m_writeCond.wait(gurad, [=] { return (0 == m_readCount) && !m_isWriting; });
    m_isWriting = true;
}

void rw_lock::unlockWrite() {
    std::unique_lock<std::mutex> gurad(m_Lock);
    m_isWriting = false;
    if (0 == (--m_writeCount)) {
        m_readCond.notify_all();
    } else {
        m_writeCond.notify_one();
    }
}
} /* namespace utils*/
} /* namespace lock*/