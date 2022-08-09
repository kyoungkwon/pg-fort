#include "query/query.h"

Query::Query(const char* raw_query)
{
    raw_query_ = strdup(raw_query);

    parse_result_ = pg_query_parse(raw_query_);
    assert(parse_result_.error == nullptr);  // TODO: throw exception

    j_ = json::parse(parse_result_.parse_tree);
}

Query::~Query()
{
    pg_query_free_parse_result(parse_result_);
    free(raw_query_);
}

json& Query::Json()
{
    return j_;
}

void Query::AddAclCheck()
{
    AddAclCheck(j_);
    // print(j_, 1);
}

void Query::AddAclCheck(json& j)
{
    for (auto& [k, v] : j.items())
    {
        if (k == "SelectStmt")
        {
            AddAclCheckToSelectStmt(v);
        }
        else if (!v.is_primitive())
        {
            // TODO: add more stmt types
            AddAclCheck(v);
        }
    }
}

void Query::AddAclCheckToSelectStmt(json& j)
{
    for (auto& [k, v] : j.items())
    {
        if (k == "fromClause")
        {
            std::cout << "  found from clause" << std::endl;
            AddAclJoinToFromClause(v);
        }
        else if (k == "whereClause")
        {
            std::cout << "  found where clause" << std::endl;
            // TODO
        }
        else if (k == "targetList")
        {
            std::cout << "  found target list" << std::endl;
            // TODO
        }
        else if (!v.is_primitive())
        {
            AddAclCheckToSelectStmt(v);
        }
    }
}

void Query::AddAclJoinToFromClause(json& j)
{
    for (auto& [k, v] : j.items())
    {
        if (k == "RangeVar")
        {
            std::cout << "   found range var" << std::endl;
            auto relname = v["relname"].get<std::string>();
            std::cout << "    table_name = " << relname << std::endl;

            ctemplate::TemplateDictionary dict("example");
            dict.SetValue("TABLE_NAME", relname);
            dict.SetValue("PRINCIPAL", "kkwon");
            dict.SetIntValue("PERM_ID", 1);

            auto ref = relname;
            if (v.contains("alias"))
            {
                auto aliasname = v["alias"]["aliasname"].get<std::string>();
                std::cout << "    aliasname = " << aliasname << std::endl;
                ref = aliasname;

                dict.SetValue("TABLE_ALIAS", aliasname);
                dict.ShowSection("ALIAS");
            }
            dict.SetValue("TABLE_REF", ref);

            std::string output;
            ctemplate::ExpandTemplate("/workspace/src/lib/query/include/query/join_expr.tpl",
                                      ctemplate::DO_NOT_STRIP, &dict, &output);

            j = json::parse(output);
            return;
        }
        else if (!v.is_primitive())
        {
            AddAclJoinToFromClause(v);
        }
    }
}

void Query::print(json& j, int indent)
{
    for (auto& [k, v] : j.items())
    {
        std::cout << indent << ":" << std::setw(indent * 4);

        std::cout << k << " (" << v.type_name() << ") : ";
        if (v.is_array() || v.is_object())
        {
            std::cout << std::endl;
            print(v, indent + 1);
        }
        else
        {
            std::cout << v << std::endl;
        }
    }
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
    pbuf.data = (char*)calloc(output.size(), sizeof(char));
    memcpy(pbuf.data, output.data(), output.size());
    pbuf.len = output.size();

    // deparse into query string
    PgQueryDeparseResult deparse_result = pg_query_deparse_protobuf(pbuf);
    assert(deparse_result.error == nullptr);  // TODO: throw exception
    char* query          = deparse_result.query;
    deparse_result.query = nullptr;

    // free
    pg_query_free_deparse_result(deparse_result);
    free(pbuf.data);

    return query;
}
