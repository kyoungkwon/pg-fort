#ifndef __POSTGRESQL_PROXY_SESSION_H__
#define __POSTGRESQL_PROXY_SESSION_H__

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "common/error.h"
#include "concurrency/thread-pool.h"
#include "conn/client-conn.h"
#include "conn/request.h"
#include "conn/response.h"
#include "conn/server-conn.h"
#include "query/jsonutil.h"
#include "query/proxy-command.h"
#include "query/query-acler.h"
#include "query/query.h"
#include "schema/schema-tracker.h"
#include "state-machine/state-machine.h"

using json = nlohmann::json;

class Session : public StateMachine
{
private:
    std::unordered_map<std::string, std::string> parameters_;

    ClientConn* cl_conn_;
    ServerConn* sv_conn_;

    std::shared_ptr<PqxxConnPool>  pcp_;
    std::shared_ptr<SchemaTracker> st_;

    // declare states and actions
    State  initiate_;
    State* Initiate();

    State  prepare_to_receive_request_;
    State* PrepareToReceiveRequest();

    State  receive_request_;
    State* ReceiveRequest();

    State  apply_pre_request_plugins_;
    State* ApplyPreRequestPlugins();

    State  prepare_to_forward_request_;
    State* PrepareToForwardRequest();

    State  forward_request_;
    State* ForwardRequest();

    State  prepare_to_receive_response_;
    State* PrepareToReceiveResponse();

    State  receive_response_;
    State* ReceiveResponse();

    State  apply_post_response_plugins_;
    State* ApplyPostResponsePlugins();

    State  prepare_to_forward_response_;
    State* PrepareToForwardResponse();

    State  forward_response_;
    State* ForwardResponse();

    State  reset_context_;
    State* ResetContext();

public:
    Session(ClientConn* cl_conn, ServerConn* sv_conn, std::shared_ptr<PqxxConnPool> pcp,
            std::shared_ptr<SchemaTracker> st);
    ~Session();

    // session functor takes a state machine action
    void operator()();

    bool               IsWaiting();
    struct epoll_event GetEpollEvent();

    // TODO: improve this
    int id;

private:
    class Context
    {
    public:
        struct epoll_event ev_;
        bool               initiated_;
        bool               waiting_;
        Request            request_;
        Response           response_;
        Query              query_;
        ProxyCommand       pcommand_;
        int                errno_;

        void Reset()
        {
            auto t     = initiated_;
            *this      = {0};
            initiated_ = t;
        }
    };

    class PlugIn
    {
    public:
        PlugIn(std::function<Error()> f);
        ~PlugIn();

        Error Apply();

    private:
        std::function<Error()> f_;
    };

    class PlugInFactory
    {
    public:
        PlugInFactory(Session* s);
        ~PlugInFactory();

        // pre-request plugins
        PlugIn GetQueryPlugIn();
        PlugIn AclQueryPlugIn();
        PlugIn DropTablePlugIn();
        PlugIn EnsureNewTableHasIdPlugIn();
        PlugIn RestrictInternalTableAccessPlugIn();
        PlugIn TranslateProxyCommandPlugIn();

        // post-response plugins
        PlugIn UpdateSchemaPlugIn();

        // TODO: maybe apply all plugins pre-request in txn and commit post-response
        // TODO: capture and synchronize with client txns

    private:
        Session* s_;
    };

    friend class PlugInFactory;

    Context             context_;
    PlugInFactory       pf_;
    std::vector<PlugIn> pre_request_plugins_;
    std::vector<PlugIn> post_response_plugins_;
};

#endif
