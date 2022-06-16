#include "session/session-pool.h"

#include "session/session.h"

SessionPool::SessionPool()
{
}

SessionPool::~SessionPool()
{
}

void SessionPool::Work()
{
    Session session;
    if (job_queue_.Pop(session, 1000) && !session.IsTerminated())
    {
        // invoke action based on current state and transition to next state
        session();

        // add the session back to the queue
        job_queue_.Push(session);
    }
}
