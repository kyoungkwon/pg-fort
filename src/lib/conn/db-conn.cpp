#include "conn/db-conn.h"

DbConn::DbConn(std::string host, int port)
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

    sock_addr_.sin_family = PF_INET;
    sock_addr_.sin_port   = htons(port_);

    int res = inet_pton(AF_INET, host_.c_str(), &sock_addr_.sin_addr);
    if (res != 1)
    {
        // TODO: throw exception
        std::cerr << "DbConn() inet_pton failed: " << res << " (errno=" << errno << ")"
                  << std::endl;
    }

    res = connect(socket_, (struct sockaddr*)&sock_addr_, sizeof(sock_addr_));
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() connect failed: " << res << " (errno=" << errno << ")" << std::endl;
    }

    res = fcntl(socket_, F_SETFL, O_NONBLOCK);
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() fcntl failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
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

DbConnFactory::DbConnFactory(std::string host, int port)
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
