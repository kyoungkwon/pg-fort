#ifndef __POSTGRESQL_PROXY_SESSION_H__
#define __POSTGRESQL_PROXY_SESSION_H__

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "concurrency/thread-pool.h"
#include "conn/client-conn.h"
#include "conn/db-conn.h"
#include "conn/request.h"
#include "conn/response.h"
#include "state-machine/state-machine.h"

class Session : public StateMachine
{
private:
    ClientConn* cl_conn_;
    DbConn*     db_conn_;

    struct
    {
        struct epoll_event ev_;
        bool               waiting_;
        Request            request_;
        Response           response_;
        int                errno_;
    } context_;

    // declare states and actions
    State  prep_recv_req;
    State* PrepRecvReq();

    State  recv_req;
    State* RecvReq();

    State  prep_fwd_req;
    State* PrepFwdReq();

    State  fwd_req;
    State* FwdReq();

    State  prep_recv_resp;
    State* PrepRecvResp();

    State  recv_resp;
    State* RecvResp();

    State  prep_fwd_resp;
    State* PrepFwdResp();

    State  fwd_resp;
    State* FwdResp();

public:
    Session(ClientConn* cl_conn, DbConn* db_conn);
    ~Session();

    // session functor takes a state machine action
    void operator()();

    bool               IsWaiting();
    struct epoll_event GetEpollEvent();

    // TODO: improve this
    int id;
};

#endif
