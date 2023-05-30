#ifndef __PG_FORT_CLIENTCONN_H__
#define __PG_FORT_CLIENTCONN_H__

#include <fcntl.h>

#include "conn/conn.h"
#include "conn/request.h"
#include "conn/response.h"

class ClientConn : public Conn
{
public:
    ClientConn(int socket);
    ~ClientConn();

    int ReceiveRequest(Request& request);
    int ForwardResponse(Response& response);
};

#endif
