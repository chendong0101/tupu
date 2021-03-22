#include <string>
#include <gtest/gtest.h>

#include "model/query.h"
#include "service/simple_sql_analyzer.h"

using namespace std;

/*
TEST(SQLAnalyzerTest, split_sql) {
    string sql = "SELECT \"籍贯\" WHERE location LIKE \"北京\" AND organization LIKE \"水务局\" AND title_alias = \"局长\" AND status = \"现任\" ORDER BY \"publish_date_timestamp\" LIMIT 2";
    query::QueryRequest *qr = sql_analyzer::sqlSplit(sql);
    EXPECT_EQ(qr->sql_order, "DESC@publish_date_timestamp");
    EXPECT_EQ(qr->limit, 2);
    ASSERT_TRUE(qr->sql_select.find("籍贯") != qr->sql_select.end());
}

TEST(SQLAnalyzerTest, build_es_query) {
    string sql = "SELECT \"籍贯\" WHERE location = \"北京\" AND organization = \"水务局\" AND title_alias = \"局长\" AND status = \"现任\" ORDER BY \"publish_date_timestamp\" LIMIT 2";
    query::QueryRequest *qr = sql_analyzer::sqlSplit(sql);
    json esRequest = sql_analyzer::buildESQuery(qr);
    EXPECT_EQ(esRequest["query"]["bool"]["must"].size(), 4);
    EXPECT_EQ(esRequest["sort"][0]["entity.publish_date_timestamp"]["order"], "DESC");
    LOG(INFO) << esRequest.dump();
}
*/