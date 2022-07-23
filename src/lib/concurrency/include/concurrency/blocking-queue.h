#ifndef __POSTGRESQL_PROXY_BLOCKINGQUEUE_H__
#define __POSTGRESQL_PROXY_BLOCKINGQUEUE_H__

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class BlockingQueue
{
private:
    std::queue<T>           q_;
    std::mutex              m_;
    std::condition_variable cv_;

public:
    void Push(const T& item)
    {
        {
            std::unique_lock<std::mutex> lock(m_);
            q_.push(item);
        }
        cv_.notify_one();
    }

    bool Pop(T& item, const int ms = 100)
    {
        auto timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(ms);

        std::unique_lock<std::mutex> lock(m_);
        while (q_.empty())
        {
            if (ms < 0)
            {
                cv_.wait(lock);
            }
            else if (cv_.wait_until(lock, timeout) == std::cv_status::timeout)
            {
                return false;
            }
        }

        item = q_.front();
        q_.pop();
        return true;
    }

    size_t Size()
    {
        return q_.size();
    }
};

#endif
