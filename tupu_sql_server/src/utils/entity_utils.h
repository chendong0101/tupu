/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef UTILS_ENTITY_H
#define UTILS_ENTITY_H

#include "thirdparty/json/nlohmann/json.hpp"
#include "message/entity.pb.h"
#include "string_utils.h"

using namespace nlohmann;

namespace utils
{
    static proto::Entity encodeEntity(const proto::Entity &entity, string encoding) {
        if (encoding.compare("utf-8") != 0 && encoding.compare("UTF-8") != 0)
        {
            proto::Entity newEntity;
            newEntity.set_kgc_id(entity.kgc_id());
            newEntity.set_entity_id(entity.entity_id());
            for (auto iter = entity.triads().begin(); iter != entity.triads().end(); ++iter)
            {
                proto::Triad *triad = newEntity.add_triads();
                triad->MergeFrom(*iter);
                string gbkKey;
                utf82gbk(iter->predicate(), gbkKey);
                triad->set_predicate(gbkKey);

                string gbkValue;
                utf82gbk(iter->value(), gbkValue);
                triad->set_value(gbkValue);
            }
            return newEntity;
        }
        return entity;
    }
    static proto::EntityContent entity2EntityContent(const proto::Entity &entity)
    {
        proto::EntityContent ec;
        ec.set_entity_id(entity.kgc_id());
        map<string, vector<proto::Triad>> docEntity;
        // select biggest priority predicate
        for_each(entity.triads().begin(), entity.triads().end(), [&docEntity](proto::Triad triad) {
            if (triad.value().empty())
            {
                return;
            }
            string key = triad.predicate();
            auto itor = docEntity.find(key);
            if (itor == docEntity.end())
            {
                docEntity[key].push_back(triad);
            }
            else
            {
                if (itor->second[0].priority() < triad.priority())
                {
                    itor->second.clear();
                    itor->second.push_back(triad);
                }
                else if (itor->second[0].priority() == triad.priority())
                {
                    itor->second.push_back(triad);
                }
            }
        });
        for (auto t : docEntity)
        {
            vector<proto::Triad> triads = t.second;
            for (auto triad : triads)
            {
                string value = triad.value();
                string predicate = triad.predicate();
                if (triad.to_kgc_id() != "-1") {
                    value = triad.to_kgc_id() + "@@@" + value;
                }
                proto::Pair *pair = ec.add_pairs();
                pair->set_key(predicate);
                pair->set_value(value);
            }
        }
        return ec;
    }

    static string proto2Json(const proto::SearchResult sr) {
        json js = R"([])"_json;
        for (auto entity : sr.entity_contents()) {
            json entityJson;
            entityJson["entity_id"] = entity.entity_id();
            json pairsJson = R"([])"_json;;
            for (auto pair : entity.pairs()) {
                json p;
                p[pair.key()] = pair.value();
                pairsJson.push_back(p);
            }
            entityJson["pairs"] = pairsJson;
            js.push_back(entityJson);
        }
        return js.dump();
    }

    static string proto2Json(const proto::FatSearchResult &fsr) {
        json js = R"([])"_json;
        for (auto entity : fsr.entities()) {
            json entityJson;
            entityJson["kgc_id"] = entity.kgc_id();
            entityJson["entity_id"] = entity.entity_id();
            json pairsJson = R"([])"_json;;
            for (auto triad : entity.triads()) {
                json p;
                p["predicate"] = triad.predicate();
                p["value"] = triad.value();
                p["to_kgc_id"] = triad.to_kgc_id();
                p["to_entity_id"] = triad.to_entity_id();
                p["source"] = triad.source();
                p["priority"] = triad.priority();
                p["op"] = triad.op();
                p["op_time"] = triad.op_time();
                pairsJson.push_back(p);
            }
            entityJson["triads"] = pairsJson;
            js.push_back(entityJson);
        }
        return js.dump();
    }
}
#endif