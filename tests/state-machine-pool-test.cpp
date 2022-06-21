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
        Design design;
        if (job_queue_.Pop(design, 1000))
        {
            if (design.IsTerminated())
            {
                std::cout << "terminated" << std::endl;
                return;
            }

            std::cout << design.GetCurrentState()->GetName() << ": " << design.GetScore()
                      << std::endl;

            // invoke action based on current state and transition to next state
            design();

            // add the session back to the queue
            job_queue_.Push(design);
        }
        else
        {
            std::cout << "empty queue" << std::endl;
        }
    }
};

void setup(DesignPool& p)
{
    Design d;
    p.Submit(d);
}

TEST(AdvancedTest, StateMachinePoolTest)
{
    DesignPool p;
    setup(p);

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
