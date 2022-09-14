#include "query/query.h"

Query::Query()
{
}

Query::Query(const Query& q)
    : j_(q.j_)
{
}

Query::Query(Query&& q) noexcept
    : j_(std::move(q.j_))
{
}

Query::~Query()
{
}

Query& Query::operator=(const Query& other)
{
    j_ = other.j_;
    return *this;
}

Query& Query::operator=(Query&& other)
{
    j_ = std::move(other.j_);
    return *this;
}

std::pair<Query, Error> Query::Parse(const char* raw_query)
{
    Query q;
    Error e;

    auto result = pg_query_parse(raw_query);
    if (result.error == nullptr)
    {
        q.j_ = json::parse(result.parse_tree);
    }

    pg_query_free_parse_result(result);

    return {std::move(q), std::move(e)};
}

json& Query::Json()
{
    return j_;
}

char* Query::ToString()
{
    // mod result
    pg_query::ParseResult mod_result;
    google::protobuf::util::JsonStringToMessage(j_.dump(), &mod_result);

    // serialize mod result
    std::string output;
    mod_result.SerializeToString(&output);

    // copy to pbuf
    PgQueryProtobuf pbuf;
    pbuf.data = const_cast<char*>(output.c_str());
    pbuf.len  = output.size();

    // deparse into query string
    PgQueryDeparseResult deparse_result = pg_query_deparse_protobuf(pbuf);
    assert(deparse_result.error == nullptr);  // TODO: throw exception
    char* query          = deparse_result.query;
    deparse_result.query = nullptr;

    // free
    pg_query_free_deparse_result(deparse_result);

    return query;
}
