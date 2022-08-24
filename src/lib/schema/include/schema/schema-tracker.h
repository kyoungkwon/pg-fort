#ifndef __POSTGRESQL_PROXY_SCHEMATRACKER_H__
#define __POSTGRESQL_PROXY_SCHEMATRACKER_H__

#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_set>

#include "conn/pqxx-conn.h"

class SchemaTracker
{
private:
    std::unordered_set<std::string> relnames_;
    std::unordered_set<std::string> excluded_;

    mutable std::shared_mutex mutex_;

public:
    SchemaTracker();
    SchemaTracker(const char* host, const char* port);
    ~SchemaTracker();

    void AddRelName(std::string relname);
    bool Exist(std::string relname);
};

#endif
