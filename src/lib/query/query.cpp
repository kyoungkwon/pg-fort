#include "query/query.h"

Query::Query(const char* raw_query)
{
    raw_query_ = strdup(raw_query);

    parse_result_ = pg_query_parse(raw_query_);
    if (parse_result_.error)
    {
        throw ParseException(parse_result_.error);
    }

    assert(parse_result_.error == nullptr);  // TODO: throw exception

    j_ = json::parse(parse_result_.parse_tree);
}

Query::~Query()
{
    pg_query_free_parse_result(parse_result_);
    free(raw_query_);
}

void Query::AddTableNames(std::vector<std::string> table_names)
{
    for (auto t : table_names)
    {
        table_names_.insert(t);
    }
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
            AddAclJoinToFromClause(v);
        }
        else if (k == "whereClause")
        {
            // TODO
            // * modify ColumnRef
            //   * checking len(fields) == 1
            //   * inserting fields[0] = table name/alias
            // AddTableRefToWhereClause(v);
        }
        else if (k == "targetList")
        {
            // TODO
            // * modify ColumnRef
            //   * checking len(fields) == 1
            //   * inserting fields[0] = table name/alias
            // AddTableRefToTargetList(v);
        }
        else if (!v.is_primitive())
        {
            // TODO: any other clauses?
            // e.g., sortClause, distinctClause, groupClause, havingClause
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
            auto relname = v["relname"].get<std::string>();

            // if relname is not a table, then ignore
            if (table_names_.find(relname) == table_names_.end())
            {
                continue;
            }

            ctemplate::TemplateDictionary dict("example");
            dict.SetValue("TABLE_NAME", relname);

            // TODO: remove hard-coded values
            dict.SetValue("PRINCIPAL", "kkwon");
            dict.SetIntValue("PERM_ID", 1);

            auto ref = relname;
            if (v.contains("alias"))
            {
                auto aliasname = v["alias"]["aliasname"].get<std::string>();
                ref            = aliasname;

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
