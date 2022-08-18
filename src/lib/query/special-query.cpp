#include "query/special-query.h"

SpecialQuery::SpecialQuery(const char* raw_query)
{
    // tokenize raw query
    std::stringstream ss(raw_query);
    std::string       s;

    while (getline(ss, s, ' '))
    {
        tokens_.push_back(s);
    }

    // check tokens[1] == "access"

    // case tokens[2]
}

SpecialQuery::~SpecialQuery()
{
}

char* SpecialQuery::Translate()
{
    return nullptr;
}