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
            delete session;  // delete terminated sessions
            return;
        }

        (*session)();  // carry out session task

        if (session->IsWaiting())
        {
            AddToWatch(session);  // add to watch
        }
        else
        {
            job_queue_.Push(session);  // add back to the queue
        }
    }
}

void SessionPool::Watch()
{
    while (!IsStopped())
    {
        std::cout << "start: " << watch_vector_.size() << std::endl;

        // check if there's any session to watch
        Session* session = nullptr;
        if (watch_vector_.empty())
        {
            if (!watch_queue_.Pop(session, 1000))
            {
                std::cout << "nothing to watch" << std::endl;
                continue;
            }
            std::cout << "something to watch" << std::endl;
            watch_vector_.emplace_back(session);
        }

        // drain queue
        while (watch_queue_.Pop(session, 0))
        {
            watch_vector_.emplace_back(session);
        }
        std::cout << "total: " << watch_vector_.size() << std::endl;

        int nfds = watch_vector_.size();

        struct pollfd fds[nfds];
        for (int i = 0; i < nfds; i++)
        {
            fds[i] = watch_vector_[i]->GetPollFd();
        }

        // TODO: maybe need a global poller to avoid long waits
        int res = poll(fds, nfds, 1000);
        if (res < 0)
        {
            // TODO: handle errno better - also proper shutdown
            std::cerr << "poll failed with res = " << res << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        else if (res == 0)
        {
            std::cout << "poll timeout" << std::endl;
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
