#ifndef __FANOUT_SERVICE_PQXXCONN_H__
#define __FANOUT_SERVICE_PQXXCONN_H__

#include <memory>
#include <pqxx/pqxx>

class PqxxConn : public pqxx::connection
{
public:
    PqxxConn(const char* host, const char* port);
    ~PqxxConn();
};

#endif
