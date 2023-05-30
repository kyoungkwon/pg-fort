#ifndef __PG_FORT_THREADPOOL_H__
#define __PG_FORT_THREADPOOL_H__

#include <functional>
#include <thread>
#include <vector>

#include "concurrency/blocking-queue.h"

template <typename J>
class ThreadPool
{
private:
    unsigned int             num_threads_;
    std::vector<std::thread> threads_;
    bool                     stopped_;

protected:
    BlockingQueue<J*> job_queue_;

    virtual void Work()
    {
        J* j = nullptr;
        if (!job_queue_.Pop(j, 1000))
        {
            (*j)();
            delete j;
        }
    }

public:
    ThreadPool(unsigned int num_threads = 0)
        : num_threads_(num_threads),
          stopped_(false)
    {
        unsigned int max = std::thread::hardware_concurrency();
        if (num_threads_ == 0 || num_threads_ > max)
        {
            num_threads_ = max;
        }
        threads_.resize(num_threads_);
    }

    virtual ~ThreadPool()
    {
    }

    virtual void Start()
    {
        for (uint32_t i = 0; i < num_threads_; i++)
        {
            threads_[i] = std::thread(
                [&]()
                {
                    while (!stopped_) Work();
                });
        }
    }

    virtual void Stop()
    {
        stopped_ = true;
        for (auto& t : threads_)
        {
            t.join();
        }
    }

    bool IsStopped()
    {
        return stopped_;
    }

    void Submit(J* j)
    {
        job_queue_.Push(j);
    }
};

#endif
