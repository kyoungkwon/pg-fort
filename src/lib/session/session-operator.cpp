#include "session/session-operator.h"

#define MAX_EVENTS 100

SessionOperator::SessionOperator(unsigned int num_threads)
    : ThreadPool(num_threads)
{
    epollfd_ = epoll_create1(0);
    if (epollfd_ == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
}

void SessionOperator::Start()
{
    ThreadPool::Start();
    watcher_ = std::thread([&]() { Watch(); });
}

void SessionOperator::Stop()
{
    ThreadPool::Stop();
    watcher_.join();
}

void SessionOperator::Work()
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
            Submit(session);  // back to the queue
        }
    }
}

void SessionOperator::Watch()
{
    struct epoll_event events[MAX_EVENTS];
    int                nfds;

    while (!IsStopped())
    {
        nfds = epoll_wait(epollfd_, events, MAX_EVENTS, 1000);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; i++)
        {
            auto s  = (Session*)events[i].data.ptr;
            auto fd = s->GetEpollEvent().data.fd;

            // session is ready to proceed, remove from epoll
            if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr) == -1)
            {
                perror("epoll_ctl: del");
                exit(EXIT_FAILURE);
            }

            // back to the queue
            Submit(s);
        }
    }
}

void SessionOperator::AddToWatch(Session* session)
{
    auto ev = session->GetEpollEvent();
    auto fd = ev.data.fd;

    // retreive session from epoll_wait
    ev.data.ptr = session;

    // add to epoll
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        perror("epoll_ctl: add");
        exit(EXIT_FAILURE);
    }
}
