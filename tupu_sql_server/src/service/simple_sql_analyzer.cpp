/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "simple_sql_analyzer.h"

#include <vector>
#include <set>
#include <iostream>

#include "log/tupu_log.h"
#include "model/query.h"
#include "property_id_service.h"
#include "utils/string_utils.h"

using namespace std;

namespace sql_analyzer
{
    enum value_type 
    {
        empty = -1,
        string_value = 0,
        set_value = 1
    };

    string vecJoin(const vector<string> &vecStr, const char *pSep = "")
    {
        if (vecStr.size() == 0)
        {
            return string();
        }
        string ret;
        for (size_t i = 0; i < vecStr.size(); ++i)
        {
            if (i > 0)
            {
                ret += pSep;
            }
            ret += vecStr[i];
        }
        return ret;
    }
    int preprocessSelectFields(const string &selectStr, vector<string> &selFields)
    {
        string sstr;
        if (selectStr[0] == '\"' && selectStr[selectStr.length() - 1] == '\"') {
            sstr = selectStr.substr(1, selectStr.length() - 2);
        } else {
            sstr = selectStr;
        }
        if (sstr.length() <= 0 || sstr.find("*") != string::npos) {
            selFields.clear();
            return 0;
        }
        utils::split(sstr, selFields, ",");

        for (int i = 0; i < selFields.size(); i++) {
            string &si = selFields.at(i);
            utils::trim(si);
        }

        return 0;
    }

    string sqlSubstr(string &sql, int start, int end, string keyWord)
    {
        int valueStartPos = start + keyWord.size();
        string valueStr = sql.substr(valueStartPos, end - valueStartPos);
        utils::trim(valueStr);
        return valueStr;
    }

    void split(string &sql, string &select,
              string &where, string &orderBy, int &limit)
    {
        int selectPos = utils::findKeyWord(sql, "SELECT");
        int wherePos = utils::findKeyWord(sql, "WHERE");
        int orderByPos = utils::findKeyWord(sql, "ORDER BY");
        int limitPos = utils::findKeyWord(sql, "LIMIT");
        int end = sql.size();
        if (limitPos >= 0)
        {
            string limitStr = sqlSubstr(sql, limitPos, end, "LIMIT");
            limit = atoi(limitStr.c_str());
            if (limit > 30) {
                limit = 30;
            }
            end = limitPos;
        }
        if (orderByPos >= 0)
        {
            orderBy = sqlSubstr(sql, orderByPos, end, "ORDER BY");
            end = orderByPos;
        }
        if (wherePos >= 0)
        {
            where = sqlSubstr(sql, wherePos, end, "WHERE");
            end = wherePos;
        }
        if (selectPos >= 0)
        {
            select = sqlSubstr(sql, selectPos, end, "SELECT");
            end = selectPos - 1;
        }
    }

    value_type preprocess(string &sql)
    {
        if (sql.length() <= 0)
            return empty;

        value_type type = string_value;
        while (true) {
            bool noneed = true;
            utils::trim(sql);
            while (sql[0] == '(' && sql[sql.length() - 1] == ')')
            {
                int i = 1;
                int t = 0;
                for (i = 1; i < sql.length() - 1; i++)
                {
                    if (sql[i] == '(')
                        t++;
                    if (sql[i] == ')')
                        t--;
                    if (t < 0)
                        break;
                }
                if (i == sql.length() - 1)
                {
                    sql = sql.substr(1, sql.length() - 2);
                    noneed = false;
                    type = set_value;
                }
                else
                {
                    break;
                }
            }
            if (noneed)
                break;
        }

        if (sql[0] == '\"' && sql[sql.length() - 1] == '\"')
        {
            if (sql.substr(1, sql.length() - 2).find("\"") == string::npos)
            {
                sql = sql.substr(1, sql.length() - 2);
            }
        }

        return type;
    }

    int preprocessOrderBy(string &orderBy)
    {
        preprocess(orderBy);
        if (orderBy.find("ASC") == 0)
        {
            string t = orderBy.substr(3);
            preprocess(t);
            orderBy = "ASC@" + t;
            return 0;
        }
        if (orderBy.find("DESC") == 0)
        {
            string t = orderBy.substr(4);
            preprocess(t);
            orderBy = "DESC@" + t;
            return 0;
        }
        if (orderBy.find("DESC") == string::npos)
        {
            string t = orderBy;
            preprocess(t);
            orderBy = "DESC@" + t;
            return 0;
        }
        return 0;
    }

    query::QueryRequest sqlSplit(string sql)
    {
        query::QueryRequest queryRequest;

        preprocess(sql);
        vector<string> selectFields;
        string select;
        string where;
        string orderBy;
        int limit = 5;

        split(sql, select, where, orderBy, limit);

        preprocessSelectFields(select, selectFields);
        preprocessOrderBy(orderBy);

        set<string> vecSelFields;
        int i = 0;
        for (string sfield : selectFields)
        {
            if (sfield.length() > 8 && sfield.find("webfact_") == 0)
            {
                sfield = sfield.substr(8);
            }
            else if (sfield.length() > 7 && sfield.find("schema_") == 0)
            {
                sfield = sfield.substr(7);
            }
            vecSelFields.insert(sfield);
        }

        queryRequest.sql_select = vecSelFields;
        queryRequest.sql_where = where;
        queryRequest.sql_order = orderBy;
        queryRequest.limit = limit;

        return queryRequest;
    }

    string calTermId(string key, string value)
    {
        utils::trim(key);
        utils::trim(value);
        PropertyIdService *propertyIdService = PropertyIdService::getInstance();
        termID32_t pId = propertyIdService->getPropertyId(key);
        if (pId < 0) {
            LOG_ERROR("property not exists: %s", key.c_str());
            return "";
        }
        termID64_t termId = utils::hash64(value);
        termId = (termId << 20) | (long)pId;
        // LOG(ERROR) << pId << "\t" << value << termId;
        char buf[17];
        sprintf(buf, "%lx\0", termId);

        return string(buf);
    }

    json genTermESQuery(string where) 
    {
        preprocess(where);
        string key, value, oper;
        string::size_type epos = where.find("=");
        string::size_type nepos = where.find("!=");
        if (nepos != string::npos) {
            key = where.substr(0, nepos);
            value = where.substr(nepos + 2);
            oper = "!=";
        } else if (epos != string::npos) {
            key = where.substr(0, epos);
            value = where.substr(epos + 1);
            oper = "=";
        } else {
            LOG_ERROR("gen termESQuery error %s", where.c_str());
            return R"({})"_json;
        }
        preprocess(key);
        value_type valueType = preprocess(value);
        if (valueType == empty) {
            LOG_ERROR("gen termESQuery error %s", where.c_str());
            return R"({})"_json;
        }
        utils::normalize(value);
        if (key == "ID") {
            json idJ;
            // idJ["ids"]["type"] = "_doc";
            idJ["ids"]["values"] = R"([])"_json;
            vector<string> ids;
            utils::split(value, ids, ",");
            for (string id : ids) {
                preprocess(id);
                idJ["ids"]["values"].push_back(id);
            }
            return idJ;
        }
        json termJ;
        vector<string> vs;
        utils::split(value, vs, ",");
        if (valueType == string_value || vs.size() == 1)
        {
            string termID = calTermId(key, value);
            if (termID.size() > 0)
            {
                termJ["term"]["entity.terms"] = termID;
            }
        } 
        else if (valueType == set_value)
        {
            termJ["terms"]["entity.terms"] = R"([])"_json;
            for (string v : vs)
            {
                preprocess(v);
                string termID = calTermId(key, v);
                if (termID.size() > 0)
                {
                    termJ["terms"]["entity.terms"].push_back(termID);
                }
            }
        }
        else
        {
            LOG_ERROR("gen termESQuery error %s", where.c_str());
            return R"({})"_json;
        }
        if (oper == "!=")
        {
            json mustNotJ;
            mustNotJ["bool"]["must_not"] = termJ;
            return mustNotJ;
        }
        return termJ;
    }

    void splitQuery2Subsentence(set<string> &subsentence, const string &ws, string oper)
    {
        int brackets = 0;
        int quotations = 0;
        string where = ws;
        preprocess(where);
        for (int pos = 0; pos < where.length(); pos++)
        {
            if (where[pos] == '(')
            {
                brackets++;
                continue;
            }
            if (where[pos] == ')')
            {
                brackets--;
                continue;
            }
            if (where[pos] == '"')
            {
                quotations = 1 - quotations;
                continue;
            }

            if (brackets == 0 && quotations == 0)
            {
                if (where.compare(pos, oper.size(), oper) == 0)
                {
                    string lwhere = where.substr(0, pos);
                    string rwhere = where.substr(pos + oper.size());
                    preprocess(lwhere);
                    subsentence.insert(lwhere);
                    splitQuery2Subsentence(subsentence, rwhere, oper);
                    return;
                }
            }
        }
        subsentence.insert(where);
    }

    json analyzeWhere(const string &where)
    {
        set<string> andSubsentence;
        set<string> orSubsentence;
        splitQuery2Subsentence(andSubsentence, where, "AND");
        splitQuery2Subsentence(orSubsentence, where, "OR");

        if (andSubsentence.size() > 1) {
            json queryJ;
            json boolJ;
            json children = R"([])"_json;
            for (set<string>::iterator iter = andSubsentence.begin(); iter != andSubsentence.end(); ++iter)
            {
                string subsentence = *iter;
                json query = analyzeWhere(subsentence);
                if (!query.empty()) {
                    children.push_back(query);
                }
            }
            boolJ["must"] = children;
            queryJ["bool"] = boolJ;
            return queryJ;
        } else if (orSubsentence.size() > 1) {
            json queryJ;
            json boolJ;
            json children = R"([])"_json;
            for (set<string>::iterator iter = orSubsentence.begin(); iter != orSubsentence.end(); ++iter)
            {
                string subsentence = *iter;
                json query = analyzeWhere(subsentence);
                if (!query.empty()) {
                    children.push_back(query);
                }
            }
            boolJ["should"] = children;
            queryJ["bool"] = boolJ;
            return queryJ;
        } else {
            return genTermESQuery(where);
        }
    }

    json buildESQuery(const query::QueryRequest &queryRequest) {
        json queryBody = R"({"_source": false})"_json;

        if (queryRequest.sql_order.size() > 0) {
            json order = R"({"mode": "max"})"_json;
            json sort;
            vector<string> orders;
            utils::split(queryRequest.sql_order, orders, "@");
            if (orders.size() != 2)
            {
                LOG_ERROR("param error: %s", queryRequest.sql_order.c_str());
                order["order"] = "DESC";
                sort["entity.weight"] = order;
            } else {
                order["order"] = orders[0];
                if (orders[1].compare("朝代权重") == 0)
                {
                    sort["entity.dynasty_weight"] = order;
                }
                else if (orders[1].compare("点击权重") == 0)
                {
                    sort["entity.click_weight"] = order;
                }
                else if (orders[1].compare("publish_date_timestamp") == 0)
                {
                    sort["entity.publish_date_timestamp"] = order;
                }
                else
                {
                    sort["entity.weight"] = order;
                }
            }
            queryBody["sort"] = json::array();
            queryBody["sort"].push_back(sort);
        }
        queryBody["size"] = queryRequest.limit;
        if (queryRequest.sql_select.size() > 0)
        {
            queryBody["size"] = 2 * queryRequest.limit;
        }
        queryBody["query"] = analyzeWhere(queryRequest.sql_where);
        return queryBody;
    }

} // namespace sql_analyzer