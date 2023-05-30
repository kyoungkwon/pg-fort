#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <string>
#include <thread>

#include "concurrency/blocking-queue.h"
#include "concurrency/thread-pool.h"
#include "state-machine/state-machine.h"

class Design : public StateMachine
{
private:
    int score;
    int inc;

    State  draft_;
    State* Submit()
    {
        score += inc;
        return &review_;
    }

    State  review_;
    State* Review()
    {
        std::cout << "reviewing... " << score << ", ";
        if (score < 50)
        {
            std::cout << "decline" << std::endl;
            return &declined_;
        }
        else if (score < 90)
        {
            std::cout << "feedback" << std::endl;
            return &feedback_;
        }
        std::cout << "approve" << std::endl;
        return &approved_;
    }

    State  feedback_;
    State* Revise()
    {
        inc = 10;
        return &draft_;
    }

    State  declined_;
    State* Restart()
    {
        inc = 30;
        return &draft_;
    }

    State  approved_;
    State* Noop()
    {
        return nullptr;
    }

public:
    Design()
        : score(0),
          inc(30),
          draft_("DRAFT", std::bind(&Design::Submit, this)),
          review_("REVIEW", std::bind(&Design::Review, this)),
          feedback_("FEEDBACK", std::bind(&Design::Revise, this)),
          declined_("DECLINED", std::bind(&Design::Restart, this)),
          approved_("APPROVED", std::bind(&Design::Noop, this))
    {
        SetInitialState(draft_);
    }

    void operator()()
    {
        std::cout << "current score = " << score << std::endl;
        TakeAction();
    }

    int GetScore()
    {
        return score;
    }
};

class DesignPool : public ThreadPool<Design>
{
public:
    DesignPool(){};
    ~DesignPool(){};

    void Work()
    {
        Design* d = nullptr;
        if (job_queue_.Pop(d, 1000))
        {
            if (d->IsTerminated())
            {
                std::cout << "terminated" << std::endl;
                delete d;
                return;
            }

            std::cout << d->GetCurrentState()->GetName() << ": " << d->GetScore() << std::endl;

            // invoke action based on current state and transition to next state
            (*d)();

            // add the session back to the queue
            job_queue_.Push(d);
        }
        else
        {
            std::cout << "empty queue" << std::endl;
        }
    }
};

TEST(AdvancedTest, StateMachinePoolTest)
{
    DesignPool p;

    auto d = new Design;
    p.Submit(d);

    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
}
