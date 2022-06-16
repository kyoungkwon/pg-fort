#include "concurrency/thread-pool.h"

ThreadPool::ThreadPool(unsigned int num_threads)
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

ThreadPool::~ThreadPool()
{
}

void ThreadPool::Start()
{
    for (uint32_t i = 0; i < num_threads_; i++)
    {
        threads_[i] = std::thread(&ThreadPool::Work, this);
    }
}

void ThreadPool::Stop()
{
    stopped_ = true;
    for (auto& t : threads_)
    {
        t.join();
    }
}

void ThreadPool::Submit(Job& job)
{
    job_queue_.Push(job);
}

void ThreadPool::Work()
{
    while (!stopped_)
    {
        Job job;
        if (!job_queue_.Pop(job, 1000))
        {
            continue;  // queue empty
        }
        job.Execute();

        // execute job

        // determine next job

        // push next job
    }
}
