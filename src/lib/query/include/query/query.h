#ifndef __POSTGRESQL_PROXY_QUERY_H__
#define __POSTGRESQL_PROXY_QUERY_H__

#include <ctemplate/template.h>
#include <google/protobuf/util/json_util.h>
#include <pg_query.h>
#include <pg_query.pb.h>
#include <pg_query/pg_query.pb-c.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Query
{
private:
    char*              raw_query_;
    PgQueryParseResult parse_result_;
    json               j_;

    void AddAclCheck(json& j);
    void AddAclCheckToSelectStmt(json& j);
    void AddAclJoinToFromClause(json& j);
    void AddTableRefToWhereClause(json& j);
    void AddTableRefToTargetList(json& j);

    void print(json& j, int indent);

public:
    Query(const char* raw_query);
    ~Query();

    json& Json();
    void  AddAclCheck();
    char* ToString();
};

#endif