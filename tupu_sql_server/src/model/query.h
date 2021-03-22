/*
 Copyright (c) 2020 Sogou, Inc.

 Author: Chen Dong (chendong01@sogou-inc.com)
*/

#ifndef _QUERY_H_
#define _QUERY_H_

#include <set>
#include <string>

using namespace std;

#define MAX_SQL_KEY_LEN 256	  //sql where中每一个属性key最大长度
#define MAX_SQL_VALUE_LEN 256 //sql where中每一个属性值最大长度
#define MAX_SQL_ATTR_NUM 64	  //select属性以及where中能使用的最多64个属性
#define MaxDocsInResult 400

#define MAX_PROP_NUM 24
#define SQL_STRING_MAX 4096

static const unsigned int MaxQueryStringLength  = 1024;

namespace query
{

	enum sql_opt_type
	{
		sql_opt_and = 0, // AND
		sql_opt_or,		 // OR
		sql_opt_like,	 // LIKE
		sql_opt_ne,		 // !=
		sql_opt_eq,		 // =
		sql_opt_lt,		 // <
		sql_opt_le,		 // <=
		sql_opt_gt,		 // >
		sql_opt_ge,		 // >=
		sql_err
	};

	struct QueryRequest
	{
		set<string> sql_select;
		string sql_order; 
		string sql_where; 
		int limit;
	};
} // namespace query

#endif /* _QUERY_H_ */
