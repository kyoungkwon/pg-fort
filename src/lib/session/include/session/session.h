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
#include "conn/db-conn.h"
#include "state-machine/state-machine.h"

class Session : public StateMachine, public Job
{
private:
    // here we keep things such as:
    //   - clConn
    //   - dbConn

    // declare states and actions
    State  waiting_request_;
    State* ReceiveRequest();

    State  request_ready_;
    State* ForwardRequest();

    State  waiting_response_;
    State* ReceiveResponse();

    State  response_ready_;
    State* ForwardResponse();

    State  error_;
    State* CloseConnection();

public:
    Session();

    // session functor takes a state machine action
    void operator()();
};

#endif
