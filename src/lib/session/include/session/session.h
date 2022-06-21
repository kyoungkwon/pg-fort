#ifndef __POSTGRESQL_PROXY_SESSION_H__
#define __POSTGRESQL_PROXY_SESSION_H__

#include <fcntl.h>
#include <sys/select.h>
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

class Session : public StateMachine, public Job
{
private:
    ClientConn* cl_conn_;
    DbConn*     db_conn_;

    struct
    {
        Request  request_;
        Response response_;
        int      errno_;
    } context_;

    // declare states and actions
    State  waiting_request_;
    State* ReceiveRequest();

    State  request_ready_;
    State* ForwardRequest();

    State  waiting_response_;
    State* ReceiveResponse();

    State  response_ready_;
    State* ForwardResponse();

    State  reset_;
    State* ResetContext();

    State  error_;
    State* CloseSession();

public:
    Session(ClientConn* cl_conn, DbConn* db_conn);

    // session functor takes a state machine action
    void operator()();

    int id;
};

#endif
