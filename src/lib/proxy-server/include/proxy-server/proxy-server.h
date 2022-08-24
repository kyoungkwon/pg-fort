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
#include <thread>

#include "conn/client-conn.h"
#include "conn/db-conn.h"
#include "schema/schema-tracker.h"
#include "session/session-operator.h"

class ProxyServer
{
private:
    std::string ip_;
    int         port_;
    int         sock_;
    sockaddr_in sock_addr_;
    socklen_t   sock_len_;
    int         flag_;

    std::shared_ptr<DbConnFactory> dbcf_;
    std::shared_ptr<SchemaTracker> st_;
    SessionOperator                so_;

public:
    ProxyServer(int port, std::shared_ptr<DbConnFactory> dbcf, std::shared_ptr<SchemaTracker> st);
    ~ProxyServer();

    void Run();
};

#endif
