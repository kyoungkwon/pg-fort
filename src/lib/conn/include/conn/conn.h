#ifndef __PG_FORT_CONN_H__
#define __PG_FORT_CONN_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "conn/buffer.h"

class Conn
{
protected:
    int socket_;

public:
    Conn(int socket)
        : socket_(socket)
    {
    }

    virtual ~Conn()
    {
        shutdown(socket_, SHUT_RDWR);
        close(socket_);
    }

    int GetSocket()
    {
        return socket_;
    }
};

#endif
