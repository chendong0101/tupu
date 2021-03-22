/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#include "sql_clause_analyzer.h"

#include <vector>
#include <set>
#include <iostream>

#include "log/tupu_log.h"
#include "utils/string_utils.h"

using namespace std;

namespace sql_analyzer
{
    void preprocess(const string &sql, int &start, int &end) {
        if (start >= end) {
            return;
        }
        while (start < end && sql[start] == ' ') {
            start++;
        }
        while (start < end && sql[end - 1] == ' ') {
            end--;
        }
        while (sql[start] == '(' && sql[end - 1] == ')') {
            start++;
            end--;
        }
        if (sql[start] == ' ' || sql[end - 1] == ' ') {
            preprocess(sql, start, end);
        }
    }

    Task *splitSubTask(const string &sql, int start, int end) {
        if (start >= end) {
            return NULL;
        }
        int brackets = 0;
        int quotations = 0;
        int selectPos = 0;
        preprocess(sql, start, end);
        bool selectFound = false;
        for (int pos = start; pos < end; pos++)
        {
            if (sql[pos] == '(')
            {
                brackets++;
                continue;
            }
            if (sql[pos] == ')')
            {
                brackets--;
                continue;
            }
            if (sql[pos] == '"')
            {
                quotations = 1 - quotations;
                continue;
            }
            if (sql.compare(pos, 6, "SELECT") == 0 && !selectFound && quotations == 0) {
                selectFound = true;
                selectPos = pos;
            }

            if (brackets == 0 && quotations == 0)
            {
                if (sql.compare(pos, 2, "OR") == 0 || sql.compare(pos, 3, "AND") == 0)
                {
                    Task *subTask = splitSubTask(sql, start, pos);
                    if (subTask != NULL) {
                        return subTask;
                    }
                    return splitSubTask(sql, pos + 3, end);
                }
            }
        }
        if (selectFound) {
            start = selectPos;
            end = end - 1;
            return new Task(sql.substr(start, end - start), start, end);
        }
        return NULL;
    }

    Task * splitSubTask(string sql) {
        int selectPos = utils::findKeyWord(sql, "SELECT");
        int wherePos = utils::findKeyWord(sql, "WHERE");
        int orderByPos = utils::findKeyWord(sql, "ORDER BY");
        int limitPos = utils::findKeyWord(sql, "LIMIT");
        int end = sql.size();
        if (orderByPos > 0) {
            end = orderByPos;
        } else if (limitPos > 0) {
            end = limitPos;
        }
        int start = wherePos + sizeof("WHERE");

        return splitSubTask(sql, wherePos + sizeof("WHERE") - 1, end);
    }
}