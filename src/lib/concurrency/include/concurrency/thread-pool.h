#ifndef __POSTGRESQL_PROXY_THREADPOOL_H__
#define __POSTGRESQL_PROXY_THREADPOOL_H__

#include <functional>
#include <thread>
#include <vector>

#include "concurrency/blocking-queue.h"

class Job
{
private:
    std::string           name_;
    std::function<void()> f_;

public:
    Job(){};
    Job(std::string name, std::function<void()> f)
        : name_(name),
          f_(f)
    {
    }

    virtual ~Job(){};

    virtual void operator()()
    {
        f_();
    };
};

class ThreadPool
{
private:
    unsigned int             num_threads_;
    std::vector<std::thread> threads_;
    bool                     stopped_;

protected:
    BlockingQueue<Job> job_queue_;

    virtual void Work();

public:
    ThreadPool(unsigned int num_threads = 0);
    virtual ~ThreadPool();

    void Start();
    void Stop();
    void Submit(Job& job);
};

#endif
