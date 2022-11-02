#ifndef __POSTGRESQL_PROXY_SCHEMATRACKER_H__
#define __POSTGRESQL_PROXY_SCHEMATRACKER_H__

#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_set>

#include "conn/pqxx-conn.h"

// TODO: this works only as a single container..
// TODO: it doesn't reflect changes when a table is dropped (i.e., RemoveRelName)
//       maybe there should only be "Exist()" and hit/miss cache?
class SchemaTracker
{
private:
    std::unordered_set<std::string> relnames_;
    std::shared_ptr<PqxxConnPool>   pcp_;
    mutable std::shared_mutex       mutex_;

public:
    SchemaTracker(std::shared_ptr<PqxxConnPool> pcp);
    ~SchemaTracker();

    void Refresh();
    void AddRelName(std::string relname);
    bool Exist(std::string relname);
};

#endif
