#ifndef __POSTGRESQL_PROXY_CLIENTCONN_H__
#define __POSTGRESQL_PROXY_CLIENTCONN_H__

#include <fcntl.h>

#include "conn/conn.h"
#include "conn/request.h"
#include "conn/response.h"

class ClientConn : public Conn
{
public:
    ClientConn(int socket);
    ~ClientConn();

    std::size_t ReceiveRequest(Request& request);
    std::size_t ForwardResponse(Response& response);
};

#endif
