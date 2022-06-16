#ifndef __POSTGRESQL_PROXY_PROXYSERVER_H__
#define __POSTGRESQL_PROXY_PROXYSERVER_H__

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <chrono>
#include <iostream>
#include <string>

#include "common/macros.h"
#include "conn/dbconn.h"
#include "session/session-pool.h"
#include "session/session.h"

class ProxyServer
{
private:
    std::string    ip_;
    int            port_;
    int            socket_;
    sockaddr_in    sock_addr_;
    socklen_t      sock_len_;
    int            flag_;
    DbConnFactory *factory_;
    SessionPool   *pool_;

public:
    ProxyServer(int port, DbConnFactory *factory);
    ~ProxyServer();

    int  Initialize();
    void Start();
};

#endif
