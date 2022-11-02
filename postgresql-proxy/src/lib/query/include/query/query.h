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
#include <utility>

#include "common/error.h"

using json = nlohmann::json;

class Query
{
private:
    bool valid_;
    json j_;

public:
    Query();
    Query(const Query& q);
    Query(Query&& q) noexcept;
    ~Query();

    Query&   operator=(const Query& other);
    Query&   operator=(Query&& other);
    explicit operator bool() const;

    static std::pair<Query, Error> Parse(const char* raw_query);

    json& Json();
    char* ToString();
};

#endif
