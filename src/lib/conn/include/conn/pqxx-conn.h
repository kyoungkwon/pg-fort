#ifndef __POSTGRESQL_PROXY_PQXX_H__
#define __POSTGRESQL_PROXY_PQXX_H__

#include <memory>
#include <pqxx/pqxx>
#include <unordered_map>

static std::string uri(const char* host, const char* port)
{
    std::stringstream ss;

    ss << "postgresql://" << std::getenv("POSTGRES_USER") << ":" << std::getenv("POSTGRES_PASSWORD")
       << "@" << host << ":" << port << "/" << std::getenv("POSTGRES_DB");

    return ss.str();
}

class PqxxConn : public pqxx::connection
{
public:
    PqxxConn(const char* host, const char* port)
        : pqxx::connection(uri(host, port).c_str())
    {
    }
};

// TODO: use conn pool
class PqxxConnPool
{
private:
    std::unordered_map<PqxxConn*, std::unique_ptr<PqxxConn>> idle_;
    std::unordered_map<PqxxConn*, std::unique_ptr<PqxxConn>> busy_;

public:
    PqxxConnPool(const char* host, const char* port, std::size_t size)
    {
        for (std::size_t i = 0; i < size; i++)
        {
            // TODO: fill idle_ with PqxxConn
        }
    }
};

#endif
