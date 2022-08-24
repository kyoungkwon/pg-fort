#ifndef __POSTGRESQL_PROXY_SESSION_H__
#define __POSTGRESQL_PROXY_SESSION_H__

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "concurrency/thread-pool.h"
#include "conn/client-conn.h"
#include "conn/db-conn.h"
#include "conn/request.h"
#include "conn/response.h"
#include "query/query.h"
#include "schema/schema-tracker.h"
#include "state-machine/state-machine.h"

class Session : public StateMachine
{
private:
    std::unordered_map<std::string, std::string> parameters_;

    ClientConn* cl_conn_;
    DbConn*     db_conn_;

    std::shared_ptr<SchemaTracker> st_;

    struct
    {
        struct epoll_event ev_;
        bool               initiated_;
        bool               waiting_;
        Request            request_;
        Response           response_;
        int                errno_;
    } context_;

    // declare states and actions
    State  initiate_;
    State* Initiate();

    State  prep_recv_req_;
    State* PrepRecvReq();

    State  recv_req_;
    State* RecvReq();

    // TODO: need to run plugins:
    //  - acl queries

    // State  req_plugin;
    // State* ReqPlugin;

    State  prep_fwd_req_;
    State* PrepFwdReq();

    State  fwd_req_;
    State* FwdReq();

    State  prep_recv_resp_;
    State* PrepRecvResp();

    State  recv_resp_;
    State* RecvResp();

    // TODO: need to run plugins:
    //  - check request type and command type
    //  - check response and command result
    //  - for create/alter/drop table:
    //    - notify schema tracker about table name

    // State  resp_plugin;
    // State* RespPlugin;

    State  prep_fwd_resp_;
    State* PrepFwdResp();

    State  fwd_resp_;
    State* FwdResp();

public:
    Session(ClientConn* cl_conn, DbConn* db_conn, std::shared_ptr<SchemaTracker> st);
    ~Session();

    // session functor takes a state machine action
    void operator()();

    bool               IsWaiting();
    struct epoll_event GetEpollEvent();

    // TODO: improve this
    int id;

private:
    class PlugIn
    {
    public:
        PlugIn(std::function<bool()> f, bool skip_on_error);
        ~PlugIn();

        bool SkipOnError();
        bool Execute();

    private:
        std::function<bool()> f_;
        bool                  skip_on_error_;
    };

    class PlugInFactory
    {
    public:
        PlugInFactory(Session* s);
        ~PlugInFactory();

        PlugIn CheckMessageTypePlugin();
        PlugIn AclQueryPlugIn();
        PlugIn CreateAclTablePlugIn();
        PlugIn DropAclTablePlugIn();

    private:
        Session* s_;
    };
    friend class PlugInFactory;

    PlugInFactory pf_;
};

#endif
