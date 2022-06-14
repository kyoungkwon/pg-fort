#include "conn/dbconn.h"

DbConn::DbConn(std::string host, int port)
    : host_(host),
      port_(port)
{
}

DbConn::~DbConn()
{
    // TODO: close?
}

int DbConn::Initialize()
{
    int retval = 0;

    socket_ = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_ < 0)
    {
        std::cerr << "dbconn::init socket failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    sock_addr_.sin_family = PF_INET;
    sock_addr_.sin_port   = htons(port_);

    retval = inet_pton(AF_INET, host_.c_str(), &sock_addr_.sin_addr);
    if (retval != 1)
    {
        std::cerr << "dbconn::init inet_pton failed: " << retval << " (errno=" << errno << ")"
                  << std::endl;
    }
    BAIL_ON_ERROR_IF(retval != 1);

    retval = connect(socket_, (struct sockaddr*)&sock_addr_, sizeof(sock_addr_));
    if (retval)
    {
        std::cerr << "dbconn::init connect failed: " << retval << " (errno=" << errno << ")"
                  << std::endl;
    }
    BAIL_ON_ERROR(retval);

cleanup:
    return retval;

error:
    // TODO: close socket?
    goto cleanup;
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
