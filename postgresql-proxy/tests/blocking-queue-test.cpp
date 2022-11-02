#include "concurrency/blocking-queue.h"

#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <string>
#include <thread>

TEST(BlockingQueueTest, MultiThreadedPushAndPop)
{
    BlockingQueue<std::string> q;

    uint32_t num_push = 10;
    uint32_t num_pop  = 7;
    uint32_t batch    = 1000;

    std::vector<std::future<void>>     pushes(num_push);
    std::vector<std::future<uint32_t>> pops(num_pop);

    // populate lambda
    auto populate = [](BlockingQueue<std::string>& q, int id, int batch)
    {
        for (uint32_t i = 0; i < batch; i++)
        {
            q.Push("hello" + std::to_string(id * batch + i));
        }
    };

    // each thread will populate a batch of strings
    for (uint32_t id = 0; id < num_push; id++)
    {
        pushes[id] = std::async(std::launch::async, populate, std::ref(q), id, batch);
    }

    // wait for all threads
    for (uint32_t id = 0; id < num_push; id++)
    {
        pushes[id].wait();
    }

    // check jobs count
    ASSERT_EQ(num_push * batch, q.Size());

    // drain lambda
    auto drain = [](BlockingQueue<std::string>& q)
    {
        uint32_t    cnt = 0;
        std::string s;
        while (q.Pop(s))
        {
            cnt++;
        }
        std::cout << "popped " << cnt << " strings" << std::endl;
        return cnt;
    };

    // threads will drain job queue
    for (uint32_t id = 0; id < num_pop; id++)
    {
        pops[id] = std::async(std::launch::async, drain, std::ref(q));
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
