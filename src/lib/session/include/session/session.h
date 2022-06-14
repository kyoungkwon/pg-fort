#ifndef __POSTGRESQL_PROXY_SESSION_H__
#define __POSTGRESQL_PROXY_SESSION_H__

#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "common/macros.h"
#include "conn/dbconn.h"

class Session
{
private:
    int     socket_;
    DbConn* dbConn_;

    // client
    // db

public:
    Session(int socket, DbConn* db_conn);
    ~Session();

    int     GetSocket();
    DbConn* GetDbConn();
    void    Serve();
    void    Serve2();

private:
    void Transmit2(int from_socket, int to_socket);
    void Transmit3(int from_socket, int to_socket);
};

// TODO: use threadpool as base class
class SessionPool
{
public:
    // TODO: pass in a condition or something to handle shutdown
    SessionPool();
    ~SessionPool();

    int Submit(Session* session);
};

#endif
