#ifndef __POSTGRESQL_PROXY_THREADPOOL_H__
#define __POSTGRESQL_PROXY_THREADPOOL_H__

#include <functional>
#include <thread>
#include <vector>

#include "blocking-queue.h"
#include "job.h"

class ThreadPool
{
private:
    unsigned int             num_threads_;
    std::vector<std::thread> threads_;
    BlockingQueue<Job>       job_queue_;
    bool                     stopped_;

    void Work();

public:
    ThreadPool(unsigned int num_threads = 0);
    ~ThreadPool();

    void Start();
    void Stop();
};

#endif
