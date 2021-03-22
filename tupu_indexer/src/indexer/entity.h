/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <set>
#include <string>
#include "thirdparty/json/nlohmann/json.hpp"
#include "message/entity.pb.h"
#include "triad.h"
#include "utils/consts.h"

using namespace std;
using json = nlohmann::json;

static set<string> stop_terms {"图片", "图片(大尺寸)", "搜狗百科", "简介", "重要性", "点击权重", "朝代权重", "朝代顺序", "SYNONYM",
                                "publish_date_timestamp", "译文", "链接", "pmining_代表作品###", "schema_合作伙伴"};
static set<termID32_t> no_segment_terms {};

class Entity
{
private:
    string kgc_id;
    string entity_id;
    unsigned long weight;
    unsigned long click_weight;
    unsigned long dynasty_weight;
    unsigned long publish_date_timestamp;
    proto::Entity pbEntity;
    set<string> termIDList;

public:
    Entity(string kgc_id, string entity_id)
    {
        this->kgc_id = kgc_id;
        this->entity_id = entity_id;
        pbEntity.set_kgc_id(kgc_id);
        pbEntity.set_entity_id(entity_id);
        weight = 0;
        click_weight = 0;
        dynasty_weight = 0;
        publish_date_timestamp = 0;
    }
    virtual ~Entity() = default;

public:
    void addTriad(Triad triad);
    void calculateTermList();

    bool isSameEntity(string id) const
    {
        return kgc_id == id;
    }

    string getKgcId() const 
    {
        return kgc_id;
    }

    string toIndexJson()
    {
        calculateTermList();
        json entity_body;
        entity_body["id"] = kgc_id;
        entity_body["weight"] = weight;
        entity_body["click_weight"] = click_weight;
        entity_body["dynasty_weight"] = dynasty_weight;
        entity_body["publish_date_timestamp"] = publish_date_timestamp;
        entity_body["terms"] = termIDList;
        json entity;
        entity["entity"] = entity_body;
        return entity.dump();
    }

    string serialize() const
    {
        string s;
        pbEntity.SerializeToString(&s);
        return s;
    }
};

#endif
