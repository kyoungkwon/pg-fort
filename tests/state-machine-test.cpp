#include "state-machine/state-machine.h"

#include <gtest/gtest.h>

#include <iostream>

class Design : public StateMachine
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

    int GetScore()
    {
        return score;
    }
};

TEST(StateMachineTest, DesignReviewProcess)
{
    std::cout << "starting state machine test" << std::endl;

    Design d;

    // DRAFT(score=0)
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("DRAFT", d.GetCurrentState()->GetName());
    ASSERT_EQ(0, d.GetScore());

    // REVIEW(score=30)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("REVIEW", d.GetCurrentState()->GetName());
    ASSERT_EQ(30, d.GetScore());

    // DECLINED(score=30)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("DECLINED", d.GetCurrentState()->GetName());
    ASSERT_EQ(30, d.GetScore());

    // DRAFT(score=30)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("DRAFT", d.GetCurrentState()->GetName());
    ASSERT_EQ(30, d.GetScore());
    
    // REVIEW(score=60)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("REVIEW", d.GetCurrentState()->GetName());
    ASSERT_EQ(60, d.GetScore());
    
    // FEEDBACK(score=60)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("FEEDBACK", d.GetCurrentState()->GetName());
    ASSERT_EQ(60, d.GetScore());
    
    // DRAFT(score=60)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("DRAFT", d.GetCurrentState()->GetName());
    ASSERT_EQ(60, d.GetScore());
    
    // REVIEW(score=70)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("REVIEW", d.GetCurrentState()->GetName());
    ASSERT_EQ(70, d.GetScore());
    
    // FEEDBACK(score=70)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("FEEDBACK", d.GetCurrentState()->GetName());
    ASSERT_EQ(70, d.GetScore());
    
    // DRAFT(score=70)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("DRAFT", d.GetCurrentState()->GetName());
    ASSERT_EQ(70, d.GetScore());
    
    // REVIEW(score=80)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("REVIEW", d.GetCurrentState()->GetName());
    ASSERT_EQ(80, d.GetScore());
    
    // FEEDBACK(score=80)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("FEEDBACK", d.GetCurrentState()->GetName());
    ASSERT_EQ(80, d.GetScore());
    
    // DRAFT(score=80)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("DRAFT", d.GetCurrentState()->GetName());
    ASSERT_EQ(80, d.GetScore());
    
    // REVIEW(score=90)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("REVIEW", d.GetCurrentState()->GetName());
    ASSERT_EQ(90, d.GetScore());
    
    // APPROVED(score=90)
    d.TakeAction();
    ASSERT_FALSE(d.IsTerminated());
    ASSERT_EQ("APPROVED", d.GetCurrentState()->GetName());
    ASSERT_EQ(90, d.GetScore());

    // terminated
    d.TakeAction();
    ASSERT_TRUE(d.IsTerminated());
}
