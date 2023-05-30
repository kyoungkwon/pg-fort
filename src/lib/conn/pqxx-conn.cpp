#include "conn/pqxx-conn.h"

static std::string uri(const char* host, const char* port)
{
    std::stringstream ss;

    ss << "postgresql://" << std::getenv("POSTGRES_USER") << ":" << std::getenv("POSTGRES_PASSWORD")
       << "@" << host << ":" << port << "/" << std::getenv("POSTGRES_DB");

    return ss.str();
}

PqxxConn::PqxxConn(const char* host, const char* port)
    : pqxx::connection(uri(host, port).c_str()),
      pool_(nullptr)
{
}

PqxxConn::~PqxxConn()
{
    if (pool_)
    {
        pool_->Release();
    }
}

PqxxConnPool::PqxxConnPool(const char* host, const char* port, std::size_t size)
    : host_(host),
      port_(port),
      size_(size)
{
    for (std::size_t i = 0; i < size_; i++)
    {
        auto c   = std::make_unique<PqxxConn>(host_, port_);
        c->pool_ = this;
        conns_.Push(std::move(c));
    }
}

PqxxConnPool::~PqxxConnPool()
{
}

std::unique_ptr<PqxxConn> PqxxConnPool::Acquire(const int ms)
{
    std::unique_ptr<PqxxConn> c;
    if (!conns_.Pop(c, ms))
    {
        return nullptr;
    }
    return c;
}

void PqxxConnPool::Release()
{
    if (size_ > conns_.Size())
    {
        auto c   = std::make_unique<PqxxConn>(host_, port_);
        c->pool_ = this;
        conns_.Push(std::move(c));
    }
}
