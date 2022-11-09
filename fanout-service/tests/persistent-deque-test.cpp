#include "persistent-deque/persistent-deque.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <future>
#include <vector>

namespace testing
{
namespace gmock_matchers_test
{

TEST(PersistentDequeTest, Simple)
{
    PersistentDeque pdq("simple");

    // push back 5 to 10
    for (int i = 5; i < 10; i++)
    {
        json j = {
            {"foo", "bar"},
            {"baz",     i}
        };

        std::cout << j.dump() << "\n";

        auto err = pdq.PushBack(j);
        ASSERT_FALSE(err);
    }

    // push front 4 to 0
    for (int i = 4; i >= 0; i--)
    {
        json j = {
            {"foo", "bar"},
            {"baz",     i}
        };

        std::cout << j.dump() << "\n";

        auto err = pdq.PushFront(j);
        ASSERT_FALSE(err);
    }

    // size = 10
    auto [s, err] = pdq.Size();
    ASSERT_FALSE(err);
    ASSERT_EQ(10, s);

    // first 3 fronts should be 0, 1, 2
    for (int i = 0; i < 3; i++)
    {
        auto [e, err] = pdq.Front();
        ASSERT_FALSE(err);
        ASSERT_EQ(i, e.Data()["baz"]);

        err = e.Pop();
        ASSERT_FALSE(err);
    }

    // first 3 backs should be 9, 8, 7
    for (int i = 9; i >= 7; i--)
    {
        auto [e, err] = pdq.Back();
        ASSERT_FALSE(err);
        ASSERT_EQ(i, e.Data()["baz"]);

        err = e.Pop();
        ASSERT_FALSE(err);
    }

    // size = 4
    std::tie(s, err) = pdq.Size();
    ASSERT_FALSE(err);
    ASSERT_EQ(4, s);

    // if not popped, front and back will always be the same
    for (int i = 0; i < 5; i++)
    {
        auto [e, err] = pdq.Front();
        ASSERT_FALSE(err);
        ASSERT_EQ(3, e.Data()["baz"]);

        // can't hold more than one element at once, release first
        e.Release();

        std::tie(e, err) = pdq.Back();
        ASSERT_FALSE(err);
        ASSERT_EQ(6, e.Data()["baz"]);
    }

    // size = 4
    std::tie(s, err) = pdq.Size();
    ASSERT_FALSE(err);
    ASSERT_EQ(4, s);

    // pop until empty
    auto remaining = s;
    while (true)
    {
        std::cout << "remaining: " << remaining << "\n";

        auto [s, err] = pdq.Size();
        ASSERT_FALSE(err);
        ASSERT_EQ(remaining--, s);

        if (s > 0)
        {
            auto [e, err] = pdq.Front();
            ASSERT_FALSE(err);

            err = e.Pop();
            ASSERT_FALSE(err);
        }
        else
        {
            break;
        }
    }
}

TEST(PersistentDequeTest, Empty)
{
    PersistentDeque pdq("empty");

    auto [e, err] = pdq.Front();
    ASSERT_TRUE(err);

    std::tie(e, err) = pdq.Back();
    ASSERT_TRUE(err);
}

TEST(PersistentDequeTest, Concurrency)
{
    auto pdq_name = "concurrency";

    uint32_t num_push = 10;
    uint32_t num_pop  = 7;
    uint32_t batch    = 1000;

    std::vector<std::future<void>>     pushes(num_push);
    std::vector<std::future<uint32_t>> pops(num_pop);

    // populate lambda
    auto populate = [](std::string name, int id, int batch, bool back)
    {
        try
        {
            std::cout << "populate (" << id << ") creating deque\n";
            PersistentDeque q(name);

            std::cout << "populate (" << id << ") uses ";
            std::function<Error(json&)> f;
            if (back)
            {
                std::cout << "PushBack\n";
                f = std::bind(&PersistentDeque::PushBack, &q, std::placeholders::_1);
            }
            else
            {
                std::cout << "PushFront\n";
                f = std::bind(&PersistentDeque::PushFront, &q, std::placeholders::_1);
            }

            for (uint32_t i = 0; i < batch; i++)
            {
                json j = {
                    {"foo",          "bar"},
                    {"baz", id * batch + i}
                };

                auto err = f(j);
                ASSERT_FALSE(err);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "populate (" << id << ") caught exception: " << e.what() << '\n';
        }
    };

    // each thread will populate a batch of objects
    for (uint32_t id = 0; id < num_push; id++)
    {
        pushes[id] = std::async(std::launch::async, populate, pdq_name, id, batch, id % 2);
    }

    // wait for all threads
    for (uint32_t id = 0; id < num_push; id++)
    {
        pushes[id].wait();
    }

    // check object count
    PersistentDeque q(pdq_name);
    auto [s, err] = q.Size();
    ASSERT_FALSE(err);
    ASSERT_EQ(num_push * batch, s);

    // drain lambda
    auto drain = [](std::string name, int id, bool front)
    {
        uint32_t cnt = 0;
        try
        {
            std::cout << "drain (" << id << ") creating deque\n";
            PersistentDeque q(name);

            std::cout << "drain (" << id << ") uses ";
            std::function<std::pair<PersistentDeque::Element, Error>(bool)> f;
            if (front)
            {
                std::cout << "Front\n";
                f = std::bind(&PersistentDeque::Front, &q, std::placeholders::_1);
            }
            else
            {
                std::cout << "Back\n";
                f = std::bind(&PersistentDeque::Back, &q, std::placeholders::_1);
            }

            while (true)
            {
                auto [e, err] = f(false);
                if (err)
                {
                    break;
                }

                err = e.Pop();
                if (err)
                {
                    std::cout << "drain (" << id << ") pop() failed" << std::endl;
                    break;
                }

                cnt++;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "drain (" << id << ") caught exception: " << e.what() << '\n';
        }

        std::cout << "drain (" << id << ") popped " << cnt << " objects" << std::endl;
        return cnt;
    };

    // threads will drain deque
    for (uint32_t id = 0; id < num_pop; id++)
    {
        pops[id] = std::async(std::launch::async, drain, pdq_name, id, id % 2);
    }

    // wait for all threads and count total
    uint32_t total_cnt = 0;
    for (uint32_t id = 0; id < num_pop; id++)
    {
        total_cnt += pops[id].get();
    }

    // check object count
    ASSERT_EQ(num_push * batch, total_cnt);
}

}  // namespace gmock_matchers_test
}  // namespace testing
