#include "session/session.h"

Session::PlugIn::PlugIn(std::function<bool()> f, bool skip_on_error)
    : f_(f),
      skip_on_fail_(skip_on_error)
{
}

Session::PlugIn::~PlugIn()
{
}

bool Session::PlugIn::SkipOnFail()
{
    return skip_on_fail_;
}

bool Session::PlugIn::Execute()
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
            auto c    = &this->s_->context_;
            auto data = c->request_.Data();

            // is this query?
            if (data[0] != 'Q')
            {
                return false;
            }

            // get the query
            c->query_ = std::make_unique<Query>(data + 5, this->s_->st_);
            return true;
        },
        false);
}

Session::PlugIn Session::PlugInFactory::AclQueryPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // add acl check to query
            auto c = &this->s_->context_;
            c->query_->AddAclCheck();
            auto qstr = c->query_->ToString();
            auto qlen = strlen(qstr);
            auto mlen = qlen + 6;

            // vectorize
            std::vector<char> v(qstr, qstr + qlen + 1);
            free(qstr);

            // append message type and info
            v.insert(v.begin(), mlen);
            v.insert(v.begin(), mlen >> 8);
            v.insert(v.begin(), mlen >> 16);
            v.insert(v.begin(), mlen >> 24);
            v.insert(v.begin(), 'Q');

            // update request with acled query
            c->request_.Take(v);
            return true;
        },
        true);
}

Session::PlugIn Session::PlugInFactory::EnsureNewTableHasIdPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // check if request was "create-table" command
            auto c = &this->s_->context_;
            auto n = JsonUtil::FindNode(c->query_->Json(), "CreateStmt");
            if (n == nullptr)
            {
                return true;
            }

            // if doesn't have column named "id" then reject
            auto have_id = false;
            for (auto& [k, v] : (*n)["tableElts"].items())
            {
                // TODO: make sure "id" is NOT NULL UNIQUE or PRIMARY
                if (k == "ColumnDef" && v["colname"] == "id")
                {
                    have_id = true;
                    break;
                }
            }

            if (!have_id)
            {
                // TODO: proper exception
                throw "ID column missing!!";
            }
            return true;
        },
        true);
}

Session::PlugIn Session::PlugInFactory::RestrictInternalTableAccessPlugIn()
{
    // TODO
    return Session::PlugIn([&]() { return true; }, true);
}

Session::PlugIn Session::PlugInFactory::CreateAclTablePlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // check if request was "create-table" command
            auto c = &this->s_->context_;
            auto n = JsonUtil::FindNode(c->query_->Json(), "CreateStmt");
            if (n == nullptr)
            {
                return true;
            }

            // check response to see if it succeeded
            auto mtype = c->response_.Data()[0];
            if (mtype != 'C')
            {
                return true;
            }

            // get table name
            auto table_name = (*n)["relation"]["relname"].get<std::string>();

            // get "if not exists" flag
            auto if_not_exists = (*n).contains("if_not_exists");
            if (if_not_exists)
            {
                if_not_exists = (*n)["if_not_exists"].get<bool>();
            }

            // TODO: set {{TABLE_NAME}}_id type accordingly to the actual source type
            static const char tpl[] =
                "CREATE TABLE {{TABLE_NAME}}__acl__ ("
                "   {{TABLE_NAME}}_id   BIGINT NOT NULL,"
                "   perm_name           TEXT NOT NULL,"
                "   principal           TEXT NOT NULL,"
                "   FOREIGN KEY ({{TABLE_NAME}}_id) REFERENCES {{TABLE_NAME}} (id),"
                "   FOREIGN KEY (perm_name) REFERENCES __access_permissions__ (name)"
                ");";

            ctemplate::StringToTemplateCache("acl_table", tpl, ctemplate::DO_NOT_STRIP);
            ctemplate::TemplateDictionary dict("acl_table_dict");
            dict.SetValue("TABLE_NAME", table_name);

            std::string cmd;
            ctemplate::ExpandTemplate("acl_table", ctemplate::DO_NOT_STRIP, &dict, &cmd);

            pqxx::result r;
            {
                // get a pqxx conn from conn-pool
                auto       pqxx = (this->s_->pcp_->Acquire());
                pqxx::work w(*pqxx);

                // issue the command to create acl table
                r = w.exec(cmd);  // TODO: what if it failed?
                w.commit();       // TODO: what if client txn hasn't committed yet?
            }

            // update schema tracker with the new table
            this->s_->st_->AddRelName(table_name);
            return true;
        },
        true);
}

Session::PlugIn Session::PlugInFactory::SelectIntoTablePlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            std::cout << "this is";
            std::cout << "SelectIntoTablePlugIn";
            std::cout << std::endl;

            // TODO: check if request was "select-into" command

            // logic:
            // table_name = SelectStmt -> intoClause -> rel -> relname
            // acl_table_name = table_name + "__acl__"

            // TODO: analyse response to see if a new table was created

            // TODO: issue a chain-command to create acl table

            // TODO: update schema tracker with the new table

            return true;
        },
        true);
}

Session::PlugIn Session::PlugInFactory::DropAclTablePlugIn()
{
    // TODO
    return Session::PlugIn([&]() { return true; }, true);
}
