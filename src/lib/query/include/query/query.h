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
#include <unordered_set>
#include <vector>

using json = nlohmann::json;

class Query
{
private:
    char*                           raw_query_;
    PgQueryParseResult              parse_result_;
    json                            j_;
    std::unordered_set<std::string> table_names_;

    void AddAclCheck(json& j);
    void AddAclCheckToSelectStmt(json& j);
    void AddAclJoinToFromClause(json& j);
    void AddTableRefToWhereClause(json& j);
    void AddTableRefToTargetList(json& j);

    void print(json& j, int indent);

public:
    Query(const char* raw_query);
    ~Query();

    void  AddTableNames(std::vector<std::string> table_names);
    json& Json();
    void  AddAclCheck();
    char* ToString();
};

class ParseException : public std::exception
{
private:
    PgQueryError* error_;

public:
    ParseException(PgQueryError* error)
        : error_(error)
    {
    }

    char* what()
    {
        // TODO
        return nullptr;
    }
};

#endif
