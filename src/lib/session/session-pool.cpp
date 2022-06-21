#include "session/session-pool.h"

#include "session/session.h"

SessionPool::SessionPool(unsigned int num_threads)
    : ThreadPool(num_threads)
{
}

void SessionPool::Work()
{
    Session* session = nullptr;
    if (job_queue_.Pop(session, 1000))
    {
        // delete terminated sessions
        if (session->IsTerminated())
        {
            delete session;
            return;
        }

        // invoke action based on current state and transition to next state
        (*session)();

        // add the session back to the queue
        job_queue_.Push(session);
    }
}
