#include "session/session-pool.h"

SessionPool::SessionPool(unsigned int num_threads)
    : ThreadPool(num_threads)
{
    if (pipe(wake_fd_) < 0)
    {
        std::cerr << "pipe(wake_fd_) failed with errno = " << errno << std::endl;
        exit(1);
    }

    // HACK: place a dummy session to reserve a spot for wake_fd_
    //       this could be replaced with epoll if I can get this run on linux container
    watch_vector_.emplace_back(new Session(nullptr, nullptr));
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

        std::cout << "------------------------------------------------------------" << std::endl;

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
        // drain queue
        Session* session = nullptr;
        while (watch_queue_.Pop(session, 0))
        {
            watch_vector_.emplace_back(session);
        }

        int nfds = watch_vector_.size();

        // set wake_fd_ as fds[0]
        struct pollfd fds[nfds];
        fds[0].fd     = wake_fd_[0];
        fds[0].events = POLLIN;
        for (int i = 1; i < nfds; i++)
        {
            fds[i] = watch_vector_[i]->GetPollFd();
        }

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
            continue;  // timeout
        }

        if (fds[0].revents)
        {
            char buf;
            read(wake_fd_[0], &buf, 1);
            continue;
        }

        // ignore i = 0 (it's wake fd)
        for (int i = 1; i < nfds; i++)
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
    write(wake_fd_[1], "a", 1);
}
