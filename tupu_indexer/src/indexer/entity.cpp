
#include "entity.h"

#include <string>
#include <glog/logging.h>
#include "utils/string_utils.h"
#include "message/entity.pb.h"
#include "property_id_service.h"
#include "triad.h"

using namespace std;

void Entity::addTriad(Triad triad)
{
    proto::Triad *pbTriad = pbEntity.add_triads();
    pbTriad->set_predicate(triad.predicate);
    pbTriad->set_value(triad.value);
    pbTriad->set_to_entity_id(triad.to_entity_id);
    pbTriad->set_to_kgc_id(triad.to_kgc_id);
    pbTriad->set_source(triad.source);
    pbTriad->set_priority(triad.priority);
    pbTriad->set_op(triad.op);
    pbTriad->set_op_time(triad.op_time);
    string predicate = triad.predicate;
    string value = triad.value;
    if (utils::isUnsignedDigit(value))
    {
        if ("重要性" == predicate)
            weight = stoul(value);
        if ("朝代权重" == predicate)
            dynasty_weight = stoul(value);
        if ("点击权重" == predicate)
            click_weight = stoul(value);
        if ("publish_date_timestamp" == predicate)
            publish_date_timestamp = stoul(value);
    }
}

void Entity::calculateTermList()
{
    map<string, vector<proto::Triad>> docEntity;
    // select biggest priority predicate
    for_each(pbEntity.triads().begin(), pbEntity.triads().end(), [&docEntity](proto::Triad triad) {
        if (triad.value().empty()) {
            return;
        }
        string key = triad.predicate();
        if (stop_terms.find(key) != stop_terms.end()) {
            return;
        }

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
    PropertyIdService *pidService = PropertyIdService::getInstance();
    for (auto t : docEntity)
    {
        vector<proto::Triad> triads = t.second;
        for (auto triad : triads)
        {
            string value = triad.value();
            string property = triad.predicate();
            if (property.find("webfact_", 0) == 0)
            {
                property = property.substr(strlen("webfact_"));
            }
            else if (property.rfind("schema_", 0) == 0)
            {
                property = property.substr(strlen("schema_"));
            }
            int32_t propId = pidService->getPropertyId(property);
            if (propId <= 0)
            {
                //LOG(INFO) << "get property id error" << property;
                continue;
            }
            utils::normalize(value);
            termID64_t termId = utils::hash64(value);
            termId = (termId << 20) | (long)propId;
            char buf[17];
            sprintf(buf, "%lx\0", termId);
            termIDList.insert(string(buf));
        }
    }
}