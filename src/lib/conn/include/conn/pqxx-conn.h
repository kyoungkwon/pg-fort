#ifndef __POSTGRESQL_PROXY_PQXXCONN_H__
#define __POSTGRESQL_PROXY_PQXXCONN_H__

#include <memory>
#include <pqxx/pqxx>

#include "concurrency/blocking-queue.h"

// forward declaration for circular dependecy resolution
class PqxxConnPool;

class PqxxConn : public pqxx::connection
{
    friend PqxxConnPool;

private:
    PqxxConnPool* pool_;

public:
    PqxxConn(const char* host, const char* port);
    ~PqxxConn();
};

class PqxxConnPool
{
private:
    const char* host_;
    const char* port_;
    std::size_t size_;

    BlockingQueue<std::unique_ptr<PqxxConn>> conns_;

public:
    PqxxConnPool(const char* host, const char* port, std::size_t size);
    ~PqxxConnPool();

    std::unique_ptr<PqxxConn> Acquire(const int ms = 500);
    void                      Release();
};

#endif
