#ifndef __POSTGRESQL_PROXY_DBCONN_H__
#define __POSTGRESQL_PROXY_DBCONN_H__

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
#include <string>

#include "conn/conn.h"
#include "conn/request.h"
#include "conn/response.h"

class ServerConn : public Conn
{
private:
    const char* host_;
    const char* port_;
    sockaddr_in sock_addr_;

public:
    ServerConn(const char* host, const char* port);
    ~ServerConn();

    int ForwardRequest(Request& request);
    int ReceiveResponse(Response& response);
};

class ServerConnFactory
{
private:
    const char* host_;
    const char* port_;

public:
    ServerConnFactory(const char* host, const char* port);
    ~ServerConnFactory();

    ServerConn* CreateServerConn();
};

#endif
