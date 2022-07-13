#ifndef __POSTGRESQL_PROXY_SESSIONPOOL_H__
#define __POSTGRESQL_PROXY_SESSIONPOOL_H__

#include <sys/poll.h>

#include <vector>

#include "concurrency/blocking-queue.h"
#include "concurrency/thread-pool.h"
#include "session/session.h"

class SessionPool : public ThreadPool<Session>
{
private:
    std::thread             watcher_;
    BlockingQueue<Session*> watch_queue_;
    std::vector<Session*>   watch_vector_;

    void Watch();
    void AddToWatch(Session* session);

public:
    SessionPool(unsigned int num_threads = 0);

    void Start();
    void Stop();
    void Work();
};

#endif
