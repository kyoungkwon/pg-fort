#include "session/session-pool.h"

SessionPool::SessionPool(unsigned int num_threads)
    : ThreadPool(num_threads)
{
}

void SessionPool::Start()
{
    ThreadPool::Start();
    watcher_ = std::thread([&]() { Watch(); });
}

void SessionPool::Stop()
{
    ThreadPool::Stop();
    watcher_.join();
}

void SessionPool::Work()
{
    Session* session = nullptr;
    if (job_queue_.Pop(session, 1000))
    {
        if (session->IsTerminated())
        {
            // delete terminated sessions
            delete session;
        }

        // carry out session task
        (*session)();

        if (session->IsWaiting())
        {
            // add to watch
            AddToWatch(session);
        }
        else
        {
            // add back to the queue
            job_queue_.Push(session);
        }
    }
}

void SessionPool::Watch()
{
    int timeout = 1 * 1000;  // 1 seconds

    while (!IsStopped())
    {
        // drain queue
        Session* session = nullptr;
        while (watch_queue_.Pop(session, 0))
        {
            watch_vector_.emplace_back(session);
        }

        int nfds = watch_vector_.size();

        struct pollfd fds[nfds];
        for (int i = 0; i < nfds; i++)
        {
            fds[i] = watch_vector_[i]->GetPollFd();
        }

        // TODO: maybe need a global poller to avoid long waits
        int res = poll(fds, nfds, timeout);
        if (res < 0)
        {
            // TODO: handle errno better - also proper shutdown
            std::cerr << "poll failed with res = " << res << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        else if (res == 0)
        {
            continue;  // timeout
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents)
            {
                job_queue_.Push(watch_vector_[i]);
                watch_vector_.erase(watch_vector_.begin() + i);
                i--;
                nfds--;
            }
        }
    }
}

void SessionPool::AddToWatch(Session* session)
{
    watch_queue_.Push(session);
}
