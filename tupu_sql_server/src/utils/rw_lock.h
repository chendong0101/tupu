/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef UTILS_RW_LOCK__H
#define UTILS_RW_LOCK__H

#include <mutex>
#include <condition_variable>

namespace utils{
namespace lock{

class rw_lock {
 public:
    rw_lock() : m_readCount(0) , m_writeCount(0) , m_isWriting(false) {}
    virtual ~rw_lock() = default;

    void lockWrite();
    void unlockWrite();
    void lockRead();
    void unlockRead();

 private:
    volatile int m_readCount;
    volatile int m_writeCount;
    volatile bool m_isWriting;
    std::mutex m_Lock;
    std::condition_variable m_readCond;
    std::condition_variable m_writeCond;
};

class rguard {
 public:
    rguard(rw_lock& lock) : m_lock(lock) {
        m_lock.lockRead();
    }
    virtual ~rguard() {
        m_lock.unlockRead();
    }

 private:
    rguard(const rguard&);
    rguard& operator=(const rguard&);

 private:
    rw_lock &m_lock;
};


class wguard {
 public:
    explicit wguard(rw_lock& lock) : m_lock(lock) {
        m_lock.lockWrite();
    }
    virtual ~wguard() {
        m_lock.unlockWrite();
    }

 private:
    wguard(const wguard&);
    wguard& operator=(const wguard&);

 private:
  rw_lock& m_lock;
};

} /* namespace utils*/
} /* namespace lock*/
#endif  // UTILS_RW_LOCK__H