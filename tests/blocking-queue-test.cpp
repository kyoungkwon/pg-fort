#include "concurrency/blocking-queue.h"

#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <string>
#include <thread>

#include "concurrency/job.h"

namespace testing
{
namespace gmock_matchers_test
{

TEST(BlockingQueueTest, MultiThreadedPushAndPop)
{
    std::cout << "starting blocking-queue-test" << std::endl;

    BlockingQueue<Job> jq;

    uint32_t num_push = 10;
    uint32_t num_pop  = 7;
    uint32_t batch    = 1000;

    std::vector<std::future<void>>     pushes(num_push);
    std::vector<std::future<uint32_t>> pops(num_pop);

    // populate lambda
    auto populate = [](BlockingQueue<Job>& jq, int id, int batch)
    {
        for (uint32_t i = 0; i < batch; i++)
        {
            Job j;
            j.name_ = "hello" + std::to_string(id * batch + i);
            jq.Push(j);
        }
    };

    // each thread will populate a batch of jobs
    for (uint32_t id = 0; id < num_push; id++)
    {
        pushes[id] = std::async(std::launch::async, populate, std::ref(jq), id, batch);
    }

    // wait for all threads
    for (uint32_t id = 0; id < num_push; id++)
    {
        pushes[id].wait();
    }

    // check jobs count
    ASSERT_EQ(num_push * batch, jq.Size());

    // drain lambda
    auto drain = [](BlockingQueue<Job>& jq)
    {
        uint32_t cnt = 0;
        Job j;
        while (jq.Pop(j))
        {
            cnt++;
        }
        std::cout << "popped " << cnt << " jobs" << std::endl;
        return cnt;
    };

    // threads will drain job queue
    for (uint32_t id = 0; id < num_pop; id++)
    {
        pops[id] = std::async(std::launch::async, drain, std::ref(jq));
    }

    // wait for all threads and count total
    uint32_t total_cnt = 0;
    for (uint32_t id = 0; id < num_pop; id++)
    {
        total_cnt += pops[id].get();
    }

    // check jobs count
    ASSERT_EQ(num_push * batch, total_cnt);
}
}  // namespace gmock_matchers_test
}  // namespace testing
