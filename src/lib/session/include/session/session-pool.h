#ifndef __POSTGRESQL_PROXY_SESSIONPOOL_H__
#define __POSTGRESQL_PROXY_SESSIONPOOL_H__

#include "concurrency/thread-pool.h"
#include "session/session.h"

class SessionPool : public ThreadPool
{
public:
    SessionPool();
    ~SessionPool();

    void Work();
};

#endif
