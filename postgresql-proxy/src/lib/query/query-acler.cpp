#include "query/query-acler.h"

QueryAcler::QueryAcler(std::shared_ptr<SchemaTracker> st)
    : st_(st)
{
}

void QueryAcler::Acl(Query& q)
{
    Acl(q.Json());
}

void QueryAcler::Acl(json& j)
{
    for (auto& [k, v] : j.items())
    {
        if (k == "SelectStmt")
        {
            AclSelectStmt(v);
        }
        else if (!v.is_primitive())
        {
            // TODO: add more stmt types
            Acl(v);
        }
    }
}

void QueryAcler::AclSelectStmt(json& j)
{
    for (auto& [k, v] : j.items())
    {
        if (k == "fromClause")
        {
            AclSelectStmtFromClause(v);
        }
        else if (k == "intoClause")
        {
            // ignore
        }
        else if (k == "whereClause")
        {
            // TODO
            // * modify ColumnRef
            //   * checking len(fields) == 1
            //   * inserting fields[0] = table name/alias
            // AclWhereClause(v);
        }
        else if (k == "targetList")
        {
            // TODO
            // * modify ColumnRef
            //   * checking len(fields) == 1
            //   * inserting fields[0] = table name/alias
            // AclTargetList(v);
        }
        else if (!v.is_primitive())
        {
            // TODO: any other clauses?
            // e.g., sortClause, distinctClause, groupClause, havingClause
            AclSelectStmt(v);
        }
    }
}

void QueryAcler::AclSelectStmtFromClause(json& j)
{
    for (auto& [k, v] : j.items())
    {
        if (k == "RangeVar")
        {
            auto relname = v["relname"].get<std::string>();

            // if relname is not a table, then ignore
            if (st_ == nullptr || !st_->Exist(relname))
            {
                continue;
            }

            ctemplate::TemplateDictionary dict("acl_query");
            dict.SetValue("TABLE_NAME", relname);
            dict.SetValue("OPERATION", "SELECT");

            // TODO: pass in principal list provided by token

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
            ctemplate::ExpandTemplate("/workspace/postgresql-proxy/src/lib/query/include/query/join_expr.tpl",
                                      ctemplate::DO_NOT_STRIP, &dict, &output);

            j = json::parse(output);
            return;
        }
        else if (k == "SelectStmt")
        {
            // a sub-query detected
            AclSelectStmt(v);
        }
        else if (!v.is_primitive())
        {
            AclSelectStmtFromClause(v);
        }
    }
}
