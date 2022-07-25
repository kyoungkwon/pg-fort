#include "conn/db-conn.h"

#include <cstring>

DbConn::DbConn(const char* host, const char* port)
    : Conn(socket(PF_INET, SOCK_STREAM, 0)),
      host_(host),
      port_(port)
{
    if (socket_ < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() socket failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    struct addrinfo  hints;
    struct addrinfo *addrs, *addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = PF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int res = getaddrinfo(host, port, &hints, &addrs);
    if (res != 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() getaddrinfo failed: " << res << " (errno=" << errno << ")"
                  << std::endl;
    }

    for (addr = addrs; addr != NULL; addr = addr->ai_next)
    {
        if ((res = connect(socket_, addr->ai_addr, addr->ai_addrlen)) == 0)
        {
            break;  // success
        }
    }
    freeaddrinfo(addrs);

    if (res != 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() connect failed: " << res << " (errno=" << errno << ")" << std::endl;
    }
    else
    {
        res = fcntl(socket_, F_SETFL, O_NONBLOCK);
        if (res < 0)
        {
            // TODO: throw exception
            std::cerr << "DbConn() fcntl failed: " << socket_ << " (errno=" << errno << ")"
                      << std::endl;
        }
    }
}

DbConn::~DbConn()
{
}

int DbConn::ForwardRequest(Request& request)
{
    return request.SendTo(socket_);
}

int DbConn::ReceiveResponse(Response& response)
{
    return response.RecvFrom(socket_);
}

DbConnFactory::DbConnFactory(const char* host, const char* port)
    : host_(host),
      port_(port)
{
}

DbConnFactory::~DbConnFactory()
{
}

DbConn* DbConnFactory::CreateDbConn()
{
    return new DbConn(host_, port_);
}
