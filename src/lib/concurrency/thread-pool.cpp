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
}

void ThreadPool::Work()
{
    while (!stopped_)
    {
        // auto job = job_queue_.Pop();
    }
}
