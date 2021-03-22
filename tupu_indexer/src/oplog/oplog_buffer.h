/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _OPLOG_BUFFER_
#define _OPLOG_BUFFER_

#include <iostream>
#include <mutex>
#include <string>
#include <list>

using namespace std;

typedef struct {
    string op;
    string _id;
    string entity_id;
    int32_t timestamp;
    string toString() {
        return _id + ": " + op + ": " + entity_id;
    }
} Oplog;

class OplogBuffer {
    private:
        list<Oplog> m_buffer;
        size_t m_bufferSize;
        std::mutex m_mutex;
    public:
        OplogBuffer(size_t size) : m_bufferSize(size) {}
        OplogBuffer() : m_bufferSize(1000) {}
        ~OplogBuffer() = default;

    public:
        bool isFull() {
            std::unique_lock<std::mutex> lock(this->m_mutex);
            return m_buffer.size() >= m_bufferSize;
        }
        void insert(Oplog oplog) {
            std::unique_lock<std::mutex> lock(this->m_mutex);
            this->m_buffer.push_back(oplog);
        }
        list<Oplog> getBuffer() {
            std::unique_lock<std::mutex> lock(this->m_mutex);
            list<Oplog> oldBuffer(m_buffer);
            m_buffer.clear();
            return oldBuffer;
        }
};

#endif