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

#include "schema/schema-tracker.h"

using json = nlohmann::json;

class Query
{
private:
    char*              raw_query_;
    PgQueryParseResult parse_result_;
    json               j_;

    std::shared_ptr<SchemaTracker> st_;

    void AddAclCheck(json& j);
    void AddAclCheckToSelectStmt(json& j);
    void AddAclJoinToFromClause(json& j);
    void AddTableRefToWhereClause(json& j);
    void AddTableRefToTargetList(json& j);

    void print(json& j, int indent);

public:
    Query(const char* raw_query, std::shared_ptr<SchemaTracker> st);
    ~Query();

    json& Json();
    void  AddAclCheck();
    char* ToString();
};

class ParseException : public std::runtime_error
{
private:
    PgQueryError* e_;

public:
    ParseException(PgQueryError* e)
        : e_(e),
          std::runtime_error("msg: " + std::string(e->message) + ", func: " + std::string(e->funcname) +
                             ", file: " + std::string(e->filename) + ", line: " + std::to_string(e->lineno) +
                             ", pos: " + std::to_string(e->cursorpos) + (e->context ? std::string(e->context) : ""))
    {
    }

    ~ParseException()
    {
        // copied "pg_query_free_error" from "pg_query_internal.h"
        if (e_)
        {
            free(e_->message);
            free(e_->funcname);
            free(e_->filename);
            if (e_->context)
            {
                free(e_->context);
            }
            free(e_);
        }
    }

    PgQueryError* error()
    {
        return e_;
    }
};

#endif
