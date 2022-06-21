#ifndef __POSTGRESQL_PROXY_DBCONN_H__
#define __POSTGRESQL_PROXY_DBCONN_H__

#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <string>

#include "conn/conn.h"
#include "conn/request.h"
#include "conn/response.h"

class DbConn : public Conn
{
private:
    std::string host_;
    int         port_;
    sockaddr_in sock_addr_;

public:
    DbConn(std::string host, int port);
    ~DbConn();

    std::size_t ForwardRequest(Request& request);
    std::size_t ReceiveResponse(Response& response);
};

class DbConnFactory
{
private:
    std::string host_;
    int         port_;

public:
    DbConnFactory(std::string host, int port);
    ~DbConnFactory();

    DbConn* CreateDbConn();
};

#endif
