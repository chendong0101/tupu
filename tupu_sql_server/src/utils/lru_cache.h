/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef UTILS_LRU_CACHE_H
#define UTILS_LRU_CACHE_H

#include <iostream>
#include <map>
#include <list>
#include <utility>
#include <chrono>

#include "type_define.h"
#include "message/entity.pb.h"
#include "rw_lock.h"

using namespace std::chrono;

namespace utils
{
    namespace cache
    {
        template <class Key, class Value>
        class lru_cache
        {
        public:
            typedef Key key_type;
            typedef Value value_type;
            typedef std::list<std::pair<key_type, milliseconds>> list_type;
            typedef std::map<key_type, std::pair<value_type, typename list_type::iterator>> map_type;

            lru_cache(size_t capacity, long lieftime) : m_capacity(capacity), m_lifetime(lieftime) {}

            ~lru_cache() = default;

            size_t size()
            {
                lock::rguard autoSync(m_lock);
                return m_map.size();
            }

            void insert(const key_type &key, const value_type &value)
            {
                lock::wguard autoSync(m_lock);
                milliseconds now_ms = duration_cast<milliseconds>(
                    system_clock::now().time_since_epoch());
                typename map_type::iterator i = m_map.find(key);
                if (i == m_map.end())
                {
                    if (m_map.size() >= m_capacity)
                    {
                        // evict item from the end of most recently used list
                        typename list_type::iterator j = prev(m_list.end(), 1);
                        m_map.erase(j->first);
                        m_list.erase(j);
                    }

                    m_list.push_front(std::make_pair(key, now_ms));
                    m_map[key] = std::make_pair(value, m_list.begin());
                }
                else
                {
                    typename list_type::iterator j = i->second.second;
                    m_list.erase(j);
                    m_list.push_front(std::make_pair(key, now_ms));
                    m_map[key] = std::make_pair(value, m_list.begin());
                }
            }

            bool get(const key_type &key, value_type &value)
            {
                lock::rguard autoSync(m_lock);
                typename map_type::iterator i = m_map.find(key);
                if (i == m_map.end())
                {
                    return false;
                }

                milliseconds now_ms = duration_cast<milliseconds>(
                    system_clock::now().time_since_epoch());

                typename list_type::iterator j = i->second.second;
                if (j->second + milliseconds(m_lifetime) > now_ms)
                {
                    value = i->second.first;
                    return true;
                }
                return false;
            }

            void clear()
            {
                lock::wguard autoSync(m_lock);
                m_map.clear();
                m_list.clear();
            }

        public:
            static lru_cache<termID64_t, std::string> *getESCache()
            {
                static lru_cache<termID64_t, std::string> es_cache(150000, 10 * 60 * 1000);
                return &es_cache;
            }
            static lru_cache<std::string, proto::Entity> *getRedisCache()
            {
                static lru_cache<std::string, proto::Entity> redis_cache(150000, 10 * 60 * 1000);
                return &redis_cache;
            }

        private:
            map_type m_map;
            list_type m_list;
            size_t m_capacity;
            long m_lifetime;
            lock::rw_lock m_lock;
        };

    } // namespace cache
} // namespace utils

#endif // UTILS_LRU_CACHE_H