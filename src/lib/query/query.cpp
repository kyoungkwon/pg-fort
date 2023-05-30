#include "query/query.h"

Query::Query()
    : valid_(false)
{
}

Query::Query(const Query& q)
    : valid_(q.valid_),
      j_(q.j_)
{
}

Query::Query(Query&& q) noexcept
    : valid_(std::move(q.valid_)),
      j_(std::move(q.j_))
{
}

Query::~Query()
{
}

Query& Query::operator=(const Query& other)
{
    valid_ = other.valid_;
    j_     = other.j_;
    return *this;
}

Query& Query::operator=(Query&& other)
{
    valid_ = std::move(other.valid_);
    j_     = std::move(other.j_);
    return *this;
}

Query::operator bool() const
{
    return valid_;
}

std::pair<Query, Error> Query::Parse(const char* raw_query)
{
    auto result = pg_query_parse(raw_query);
    if (result.error)
    {
        Error err = {
            {"S",                                 "ERROR"},
            {"M",                   result.error->message},
            {"R",                  result.error->funcname},
            {"F",                  result.error->filename},
            {"L",    std::to_string(result.error->lineno)},
            {"P", std::to_string(result.error->cursorpos)}
        };

        pg_query_free_parse_result(result);
        return {Query(), std::move(err)};
    }

    Query q;
    q.valid_ = true;
    q.j_     = json::parse(result.parse_tree);

    pg_query_free_parse_result(result);
    return {std::move(q), NoError};
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
