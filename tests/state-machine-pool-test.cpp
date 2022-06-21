#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <string>
#include <thread>

#include "concurrency/blocking-queue.h"
#include "concurrency/thread-pool.h"
#include "state-machine/state-machine.h"

class Design : public StateMachine, public Job
{
private:
    int score = 0;
    int inc   = 30;

    State  draft_;
    State* Submit()
    {
        score += inc;
        return &review_;
    }

    State  review_;
    State* Review()
    {
        if (score < 50)
        {
            return &declined_;
        }
        else if (score < 90)
        {
            return &feedback_;
        }
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
        : draft_("DRAFT", std::bind(&Design::Submit, this)),
          review_("REVIEW", std::bind(&Design::Review, this)),
          feedback_("FEEDBACK", std::bind(&Design::Revise, this)),
          declined_("DECLINED", std::bind(&Design::Restart, this)),
          approved_("APPROVED", std::bind(&Design::Noop, this))
    {
        SetInitialState(draft_);
    }

    void operator()()
    {
		std::cout << "(before) " << GetCurrentState()->GetName() << " -> ";
        TakeAction();
		std::cout << "(after) " << GetCurrentState()->GetName() << std::endl;
    }

    int GetScore()
    {
        return score;
    }
};

class DesignPool : public ThreadPool
{
public:
    DesignPool(){};
    ~DesignPool(){};

    void Work()
    {
        Design design;
        if (job_queue_.Pop(design, 1000) && !design.IsTerminated())
        {
            std::cout << design.GetCurrentState()->GetName() << ": " << design.GetScore()
                      << std::endl;

            // invoke action based on current state and transition to next state
            design();

            // add the session back to the queue
            job_queue_.Push(design);
        }
    }
};

TEST(AdvancedTest, StateMachinePoolTest)
{
    DesignPool p;
    Design     d;

    p.Submit(d);
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
    p.Work();
}
