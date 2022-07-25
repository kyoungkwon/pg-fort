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

class DbConn : public Conn
{
private:
    const char* host_;
    const char* port_;
    sockaddr_in sock_addr_;

public:
    DbConn(const char* host, const char* port);
    ~DbConn();

    int ForwardRequest(Request& request);
    int ReceiveResponse(Response& response);
};

class DbConnFactory
{
private:
    const char* host_;
    const char* port_;

public:
    DbConnFactory(const char* host, const char* port);
    ~DbConnFactory();

    DbConn* CreateDbConn();
};

#endif
