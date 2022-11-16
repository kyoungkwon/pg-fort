#include "conn/pqxx-conn.h"

static std::string uri(const char* host, const char* port)
{
    std::stringstream ss;

    ss << "postgresql://" << std::getenv("POSTGRES_USER") << ":" << std::getenv("POSTGRES_PASSWORD")
       << "@" << host << ":" << port << "/" << std::getenv("POSTGRES_DB");

    return ss.str();
}

PqxxConn::PqxxConn(const char* host, const char* port)
    : pqxx::connection(uri(host, port).c_str())
{
}

PqxxConn::~PqxxConn()
{
}
