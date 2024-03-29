#include "conn/client-conn.h"

ClientConn::ClientConn(int socket)
    : Conn(socket)
{
    if (fcntl(socket_, F_SETFL, O_NONBLOCK) < 0)
    {
        // TODO: throw exception
        std::cerr << "ClientConn() fcntl failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }
}

ClientConn::~ClientConn()
{
}

int ClientConn::ReceiveRequest(Request& request)
{
    return request.RecvFrom(socket_);
}

int ClientConn::ForwardResponse(Response& response)
{
    return response.SendTo(socket_);
}
