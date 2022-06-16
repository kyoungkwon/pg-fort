#include "conn/db-conn.h"

DbConn::DbConn(std::string host, int port)
    : host_(host),
      port_(port)
{
    int res = 0;

    socket_ = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_ < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() socket failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    sock_addr_.sin_family = PF_INET;
    sock_addr_.sin_port   = htons(port_);

    res = inet_pton(AF_INET, host_.c_str(), &sock_addr_.sin_addr);
    if (res != 1)
    {
        // TODO: throw exception
        std::cerr << "DbConn() inet_pton failed: " << res << " (errno=" << errno << ")"
                  << std::endl;
    }

    res = connect(socket_, (struct sockaddr*)&sock_addr_, sizeof(sock_addr_));
    if (res)
    {
        // TODO: throw exception
        std::cerr << "DbConn() connect failed: " << res << " (errno=" << errno << ")"
                  << std::endl;
    }
}

DbConn::~DbConn()
{
    // TODO: close?
}

int DbConn::GetSocket()
{
    return socket_;
}

DbConnFactory::DbConnFactory(int port)
    : port_(port)
{
}

DbConnFactory::~DbConnFactory()
{
}

DbConn* DbConnFactory::CreateDbConn()
{
    return new DbConn("127.0.0.1", port_);
}
