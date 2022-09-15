#include "session/session.h"

Session::PlugIn::PlugIn(std::function<Error()> f)
    : f_(f)
{
}

Session::PlugIn::~PlugIn()
{
}

Error Session::PlugIn::Apply()
{
    return f_();
}

Session::PlugInFactory::PlugInFactory(Session* s)
    : s_(s)
{
}

Session::PlugInFactory::~PlugInFactory()
{
}

Session::PlugIn Session::PlugInFactory::GetQueryPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // get request data
            auto c    = &s_->context_;
            auto data = c->request_.Data();

            // is request a query?
            if (data[0] == 'Q')
            {
                // ignore error - let the server produce a proper error response
                std::tie(c->query_, std::ignore) = Query::Parse(data + 5);
            }
            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::AclQueryPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // is request a query?
            auto c = &s_->context_;
            if (!c->query_)
            {
                return NoError;
            }

            // add acl check to query
            // c.query_->AddAclCheck();
            auto qstr = c->query_.ToString();

            std::cout << "Acled query:\n\t" << qstr << std::endl;

            // update request with acled query
            c->request_.SetQuery(qstr);
            free(qstr);
            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::DropTablePlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // is request a query?
            auto c = &s_->context_;
            if (!c->query_)
            {
                return NoError;
            }

            // check if query was "drop-table" command
            auto n = JsonUtil::FindNode(c->query_.Json(), "DropStmt");
            if (n == nullptr || (*n)["removeType"] != "OBJECT_TABLE")
            {
                return NoError;
            }

            std::cout << "DropTablePlugIn: detected\n";

            // get acl table names
            std::vector<std::string> table_names;
            for (auto& [_, obj] : (*n)["objects"].items())
            {
                auto tn = obj["List"]["items"][0]["String"]["str"].get<std::string>();
                if (s_->st_->Exist(tn))
                {
                    table_names.push_back(tn + "__acl__");
                }
            }

            // return if there's no acl table to drop
            if (table_names.empty())
            {
                return NoError;
            }

            // create a copy of the query and build a test query
            Query test_q(c->query_);
            auto  objs = JsonUtil::FindNode(test_q.Json(), "objects");
            for (auto& tn : table_names)
            {
                objs->push_back(json({
                    {"List", {{"items", {{{"String", {{"str", tn}}}}}}}}
                }));
            }
            auto test_qstr = test_q.ToString();

            {
                // get a pqxx conn from conn-pool
                auto       pqxx = (s_->pcp_->Acquire());
                pqxx::work w(*pqxx);

                try
                {
                    w.exec(test_qstr);

                    // passed the test - add "cascade" to the query
                    std::cout << "DropTablePlugIn: dep check passed :)\n";
                    (*n)["behavior"] = "DROP_CASCADE";

                    auto qstr = c->query_.ToString();
                    std::cout << "\t" << qstr << "\n";
                    c->request_.SetQuery(qstr);
                    free(qstr);
                }
                catch (pqxx::sql_error const& e)
                {
                    // didn't pass the test - let the query fail
                    std::cout << "DropTablePlugIn: dep check failed ;(\n";
                }

                w.abort();
            }
            free(test_qstr);
            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::EnsureNewTableHasIdPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // is request a query?
            auto c = &s_->context_;
            if (!c->query_)
            {
                return NoError;
            }

            // check if query was "create-table" command
            auto n = JsonUtil::FindNode(c->query_.Json(), "CreateStmt");
            if (n == nullptr)
            {
                return NoError;
            }

            // "id" column must exist
            auto have_id = false;

            // "id" column must be NOT NULL UNIQUE or PRIMARY
            auto not_null = false;
            auto unique   = false;
            auto primary  = false;

            // inspect table
            for (auto& [_, elt] : (*n)["tableElts"].items())
            {
                // check per-column constraints
                if (elt["ColumnDef"]["colname"] == "id")
                {
                    have_id = true;
                    for (auto& [_, cstr] : elt["ColumnDef"]["constraints"].items())
                    {
                        auto contype = cstr["Constraint"]["contype"];
                        not_null |= contype == "CONSTR_NOTNULL";
                        unique |= contype == "CONSTR_UNIQUE";
                        primary |= contype == "CONSTR_PRIMARY";
                    }
                }

                // check table constraints
                if (elt["Constraint"]["contype"] == "CONSTR_PRIMARY")
                {
                    auto keys = elt["Constraint"]["keys"];
                    if (keys.size() == 1)
                    {
                        primary |= keys[0]["String"]["str"] == "id";
                    }
                }
            }

            if (!have_id)
            {
                auto msg = "column \"id\" missing";
                std::cout << msg << std::endl;
                return Error({
                    {"S",                     "ERROR"},
                    {"C",                     "42P16"},
                    {"M",                         msg},
                    {"F",       std::string(__FILE__)},
                    {"L",    std::to_string(__LINE__)},
                    {"R", "EnsureNewTableHasIdPlugIn"}
                });
            }

            if (!primary && !(not_null && unique))
            {
                auto msg = "column \"id\" is neither PRIMARY nor NOT NULL UNIQUE";
                std::cout << msg << std::endl;
                return Error({
                    {"S",                     "ERROR"},
                    {"C",                     "42P16"},
                    {"M",                         msg},
                    {"F",       std::string(__FILE__)},
                    {"L",    std::to_string(__LINE__)},
                    {"R", "EnsureNewTableHasIdPlugIn"}
                });
            }
            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::RestrictInternalTableAccessPlugIn()
{
    // TODO
    return Session::PlugIn([&]() { return NoError; });
}

Session::PlugIn Session::PlugInFactory::CreateAclTablePlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // is request a query?
            auto c = &s_->context_;
            if (!c->query_)
            {
                return NoError;
            }

            // check response to see if it succeeded
            auto mtype = c->response_.Data()[0];
            if (mtype != 'C')
            {
                return NoError;
            }

            // check if request was "create-table" command
            auto n = JsonUtil::FindNode(c->query_.Json(), "CreateStmt");
            if (n == nullptr)
            {
                return NoError;
            }

            // get table name
            auto table_name = (*n)["relation"]["relname"].get<std::string>();

            // get "if not exists" flag
            auto if_not_exists = (*n).contains("if_not_exists");
            if (if_not_exists)
            {
                if_not_exists = (*n)["if_not_exists"].get<bool>();
            }

            // TODO: set id type accordingly to the actual source type
            // TODO: add columns?
            static const char tpl[] =
                "CREATE TABLE {{TABLE_NAME}}__acl__ (\n"
                "   id          BIGINT NOT NULL,\n"
                "   permission  TEXT NOT NULL,\n"
                "   operation   TEXT NOT NULL,\n"
                "   principal   TEXT NOT NULL,\n"
                "   FOREIGN KEY (id) REFERENCES {{TABLE_NAME}} (id) ON DELETE CASCADE,\n"
                "   FOREIGN KEY (permission) REFERENCES __access_permissions__ (name) ON DELETE RESTRICT\n"
                ");";

            ctemplate::StringToTemplateCache("create_acl_table", tpl, ctemplate::DO_NOT_STRIP);
            ctemplate::TemplateDictionary dict("create_acl_table_dict");
            dict.SetValue("TABLE_NAME", table_name);

            std::string cmd;
            ctemplate::ExpandTemplate("create_acl_table", ctemplate::DO_NOT_STRIP, &dict, &cmd);

            std::cout << "Creating a new acl table:\n" << cmd << std::endl;

            // TODO: generate triggers for...

            // TODO: generate create command for per-relation acls view

            {
                // get a pqxx conn from conn-pool
                auto       pqxx = (s_->pcp_->Acquire());
                pqxx::work w(*pqxx);

                // issue the command to create acl table
                w.exec(cmd);  // TODO: what if it failed?
                w.commit();   // TODO: what if client txn hasn't committed yet?
            }

            // update schema tracker with the new table
            s_->st_->AddRelName(table_name);

            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::SelectIntoTablePlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // TODO: check if request was "select-into" command

            // logic:
            // table_name = SelectStmt -> intoClause -> rel -> relname
            // acl_table_name = table_name + "__acl__"

            // TODO: analyse response to see if a new table was created

            // TODO: issue a chain-command to create acl table

            // TODO: update schema tracker with the new table
            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::DropAclTablePlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // is request a query?
            auto c = &s_->context_;
            if (!c->query_)
            {
                return NoError;
            }

            // check response to see if it succeeded
            auto mtype = c->response_.Data()[0];
            if (mtype != 'N')  // TODO: change "NoticeResponse" to "CommandComplete"
            {
                return NoError;
            }

            // check if request was "drop-table" command
            auto n = JsonUtil::FindNode(c->query_.Json(), "DropStmt");
            if (n == nullptr || (*n)["removeType"] != "OBJECT_TABLE")
            {
                return NoError;
            }

            std::cout << "DropAclTablePlugIn: detected\n";

            // get acl table names
            std::vector<std::string> table_names;
            for (auto& [_, obj] : (*n)["objects"].items())
            {
                auto tn = obj["List"]["items"][0]["String"]["str"].get<std::string>();
                if (s_->st_->Exist(tn))
                {
                    table_names.push_back(tn + "__acl__");
                }
            }

            // return if there's no acl table to drop
            if (table_names.empty())
            {
                return NoError;
            }

            // build a new "drop-table" command
            auto qstr =
                "DROP TABLE " + std::accumulate(std::next(table_names.begin()), table_names.end(), table_names[0],
                                                [](std::string a, std::string b) { return std::move(a) + ", " + b; });

            std::cout << "\t" << qstr << "\n";

            {
                // get a pqxx conn from conn-pool
                auto       pqxx = (s_->pcp_->Acquire());
                pqxx::work w(*pqxx);

                w.exec(qstr);
                w.commit();  // TODO: what if client txn hasn't committed yet?
            }

            // TODO: change "NoticeResponse" to "CommandComplete"
            return NoError;
        });
}
