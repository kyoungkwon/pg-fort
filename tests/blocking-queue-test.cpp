#include "concurrency/blocking-queue.h"

#include <iostream>
#include <string>

#include "concurrency/job.h"

void populate(BlockingQueue<Job>& jq)
{
    for (uint32_t i = 0; i < 30; i++)
    {
        Job j;
        j.name_ = "hello" + std::to_string(i);
        jq.Push(j);
    }
}

void drain(BlockingQueue<Job>& jq)
{
    Job j;
    while (jq.Pop(j))
    {
        std::cout << "popped " << j.name_ << std::endl;
    }
    std::cout << "drain complete" << std::endl;
}

int main()
{
    std::cout << "starting blocking-queue-test" << std::endl;

    BlockingQueue<Job> jq;

    populate(jq);

    drain(jq);
}
