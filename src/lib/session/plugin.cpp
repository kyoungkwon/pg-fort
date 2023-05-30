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

Session::PlugIn Session::PlugInFactory::TranslateProxyCommandPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // get request data
            auto c    = &s_->context_;
            auto data = c->request_.Data();

            // is request a query?
            if (data[0] != 'Q')
            {
                return NoError;
            }

            // translate proxy commands
            auto [translated, err] = ProxyCommand::Translate(data + 5);
            if (err)
            {
                return err;
            }

            // update request with translated command
            c->request_.SetQuery(translated.c_str());
            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::ParseQueryPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // get request data
            auto c    = &s_->context_;
            auto data = c->request_.Data();

            // is request a query?
            if (data[0] != 'Q')
            {
                return NoError;
            }

            // is this a regular query?
            Error err;
            std::tie(c->query_, err) = Query::Parse(data + 5);
            if (!err)
            {
                return NoError;
            }

            // TODO:
            // // is this a plpgsql?
            // std::tie(c->plpgsql_, err) = Plpgsql::Parse(data + 5);
            // if (!err)
            // {
            //     return NoError;
            // }

            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::AclQueryPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // skip if superuser
            if (s_->parameters_["rolsuper"] == "t")
            {
                return NoError;
            }

            // is request a query?
            auto c = &s_->context_;
            if (!c->query_)
            {
                return NoError;
            }

            // add acl check to query
            QueryAcler(s_->st_).Acl(c->query_);
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

            // get bindings table names
            std::vector<std::string> table_names;
            for (auto& [_, obj] : (*n)["objects"].items())
            {
                auto tn = obj["List"]["items"][0]["String"]["str"].get<std::string>();
                if (s_->st_->Exist(tn))
                {
                    table_names.push_back(tn + "__access_bindings__");
                }
            }

            // return if there's bindings table to drop
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

            // get table name
            auto table_name = (*n)["relation"]["relname"].get<std::string>();

            // skip the check if it's a binding table
            if (table_name.ends_with("__access_bindings__"))
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

Session::PlugIn Session::PlugInFactory::UpdateParametersPlugIn()
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

            // check if request was "set-role" command
            auto n = JsonUtil::FindNode(c->query_.Json(), "VariableSetStmt");
            if (n == nullptr || (*n)["name"] != "role")
            {
                return NoError;
            }

            // check response to see if it succeeded
            auto mtype = c->response_.Data()[0];
            if (mtype != 'C')
            {
                return NoError;
            }

            // update user parameter
            s_->parameters_["user"] = (*n)["args"][0]["A_Const"]["val"]["String"]["str"];

            // update rolsuper parameter
            pqxx::row r;
            {
                auto       pqxx = (s_->pcp_->Acquire());
                pqxx::work w(*pqxx);
                r = w.exec1("SELECT rolsuper FROM pg_roles WHERE rolname = '" + s_->parameters_["user"] + "'");
                w.commit();
            }
            s_->parameters_["rolsuper"] = r["rolsuper"].c_str();

            std::cout << "rolsuper_rolsuper_rolsuper_rolsuper_rolsuper_rolsuper_rolsuper\n";
            std::cout << s_->parameters_["rolsuper"] << "\n";
            std::cout << "rolsuper_rolsuper_rolsuper_rolsuper_rolsuper_rolsuper_rolsuper\n";

            return NoError;
        });
}

Session::PlugIn Session::PlugInFactory::UpdateSchemaPlugIn()
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

            // update schema tracker with the new table
            s_->st_->AddRelName(table_name);

            return NoError;
        });
}
