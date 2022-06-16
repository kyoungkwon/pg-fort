#ifndef __POSTGRESQL_PROXY_JOB_H__
#define __POSTGRESQL_PROXY_JOB_H__

#include <string>

class Job
{
private:
    std::string name_;  // TODO: remove

public:
    std::string GetName();
    int         Execute();
};

#endif
