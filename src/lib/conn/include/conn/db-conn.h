#ifndef __POSTGRESQL_PROXY_DBCONN_H__
#define __POSTGRESQL_PROXY_DBCONN_H__

#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <string>

#include "conn/conn.h"

class DbConn : public Conn
{
private:
    std::string host_;
    int         port_;
    int         socket_;
    sockaddr_in sock_addr_;

public:
    DbConn(std::string host, int port);
    ~DbConn();

    int GetSocket();
};

class DbConnFactory
{
private:
    int port_;

public:
    DbConnFactory(int port);
    ~DbConnFactory();

    DbConn *CreateDbConn();
};

#endif
