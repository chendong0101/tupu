/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "full_indexer.h"

#include <unistd.h>
#include <ctime>
#include <iostream>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include "triad.h"

using namespace std;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;

DECLARE_string(mongo_uri);

void FullIndexer::run(string regex) {
    mongocxx::uri uri(FLAGS_mongo_uri);
    mongocxx::client client(uri);
    mongocxx::database db = client["tupu"];
    mongocxx::collection coll = db["triad"];

    auto matchRegex = bsoncxx::types::b_regex(regex);
    mongocxx::pipeline p{};
    p.match(document() << "kgc_id" << matchRegex << finalize);
    // p.match(document() << "kgc_id" << "a782cf1707c27ada50c19c027fff13c6" << finalize);
    // p.sort(document() << "kgc_id" << 1 << finalize);
    // p.match(document() << "$and" << open_array 
    //             << open_document
    //                 << "$expr" 
    //                 << open_document
    //                     << "$gte" << open_array << "$kgc_id" << "00000000000000000000000000000000" << close_array
    //                 << close_document
    //             <<close_document
    //             <<open_document
    //                 << "$expr" 
    //                 << open_document
    //                     << "$lt" << open_array << "$kgc_id" << "00000000000000000000000000000000" << close_array
    //                 << close_document
    //             << close_document
    //         << close_array
    //     <<finalize);
    p.project(document() << "_id" << 1
                         << "entity_id" << 1
                         << "kgc_id" << 1
                         << "predicate" << 1
                         << "value" << 1
                         << "to_entity_id" << 1
                         << "to_kgc_id" << 1
                         << "source" << 1
                         << "priority" << 1
                         << "op" << 1
                         << "op_time" << 1
                         << finalize);
    // p.group(document() << "_id" << "$entity_id"
    //     << "entity" << open_document
    //         << "$push" << open_document
    //             << "predicate" << "$predicate"
    //             << "value" << "$value"
    //             << "to_entity_id" << "$to_entity_id"
    //             << "priority" << "$priority"
    //         << close_document
    //     << close_document
    //     << finalize
    // );

    auto cursor = coll.aggregate(p, mongocxx::options::aggregate{});
    int i = 1;

    vector<Triad> indexJob;
    string currentEntityId;
    for (auto c : cursor)
    {
        Triad t;
        try {
            t.parseFromMongo(c);
        } catch (const std::exception &e) {
            LOG(ERROR) << "parse mongo error";
            continue;
        }

        if (t.kgc_id.compare(currentEntityId) != 0)
        {
            if (m_workerThreadPool->isFull())
            {
                LOG(INFO) << "job buffer is full" ;
                usleep(200 * 1000);
            }
            currentEntityId = t.kgc_id;
            if (i++ % 10000 == 0)
            {
                LOG(INFO) << "finished: " << i;
                // break;
            }
            if (!indexJob.empty())
            {
                m_workerThreadPool->submit(std::move(indexJob));
                indexJob.clear();
            }
        }
        indexJob.push_back(t);
    }
    m_workerThreadPool->submit(std::move(indexJob));
}
