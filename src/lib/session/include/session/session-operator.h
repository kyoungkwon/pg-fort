#ifndef __PG_FORT_SESSIONOPERATOR_H__
#define __PG_FORT_SESSIONOPERATOR_H__

#include <sys/epoll.h>

#include <vector>

#include "concurrency/blocking-queue.h"
#include "concurrency/thread-pool.h"
#include "session/session.h"

class SessionOperator : public ThreadPool<Session>
{
private:
    int         epollfd_;
    std::thread watcher_;

    void Watch();
    void AddToWatch(Session* session);

public:
    SessionOperator(unsigned int num_threads = 0);

    void Start();
    void Stop();
    void Work();
};

#endif
