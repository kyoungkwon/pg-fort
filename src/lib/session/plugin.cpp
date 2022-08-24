#include "session/session.h"

Session::PlugIn::PlugIn(std::function<bool()> f, bool skip_on_error)
    : f_(f),
      skip_on_error_(skip_on_error)
{
}

Session::PlugIn::~PlugIn()
{
}

bool Session::PlugIn::SkipOnError()
{
    return skip_on_error_;
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

Session::PlugIn Session::PlugInFactory::AclQueryPlugIn()
{
    return Session::PlugIn(
        [&]()
        {
            // get request data
            auto s    = this->s_;
            auto data = s->context_.request_.Data();
            auto size = s->context_.request_.Size();

            // is this query?
            if (data[0] != 'Q')
            {
                return true;
            }

            // convert data into query
            Query q(data + 5, s->st_);

            // add acl check to query
            q.AddAclCheck();
            auto qstr = q.ToString();
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
            s->context_.request_.Take(v);

            return true;
        },
        false);
}

Session::PlugIn Session::PlugInFactory::CreateAclTablePlugIn()
{
    return Session::PlugIn(
        []()
        {
            std::cout << "this is";
            std::cout << "CreateAclTablePlugIn";
            std::cout << std::endl;

            // TODO: issue a chain-command to create acl table

            // TODO: update schema tracker with the new table

            return true;
        },
        false);
}

Session::PlugIn Session::PlugInFactory::DropAclTablePlugIn()
{
    return Session::PlugIn(
        []()
        {
            std::cout << "this is";
            std::cout << "DropAclTablePlugIn";
            std::cout << std::endl;
            return true;
        },
        false);
}
