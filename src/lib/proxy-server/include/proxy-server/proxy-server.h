#ifndef __PG_FORT_PROXYSERVER_H__
#define __PG_FORT_PROXYSERVER_H__

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
#include "conn/server-conn.h"
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

    std::shared_ptr<ServerConnFactory> scf_;
    std::shared_ptr<PqxxConnPool>      pcp_;
    std::shared_ptr<SchemaTracker>     st_;
    SessionOperator                    so_;

public:
    ProxyServer(int port, std::shared_ptr<ServerConnFactory> scf, std::shared_ptr<PqxxConnPool> pcp,
                std::shared_ptr<SchemaTracker> st);
    ~ProxyServer();

    void Run();
};

#endif
