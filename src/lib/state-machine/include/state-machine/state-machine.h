#ifndef __POSTGRESQL_PROXY_STATEMACHINE_H__
#define __POSTGRESQL_PROXY_STATEMACHINE_H__

#include <iostream>
#include <set>
#include <string>

class State
{
protected:
    std::string             name_;
    std::function<State*()> action_;

public:
    State(std::string name, std::function<State*()> action)
        : name_(name),
          action_(action)
    {
    }

    std::string GetName()
    {
        return name_;
    }

    State* TakeAction()
    {
        if (!action_)
        {
            std::cerr << "No action - throw exception here" << std::endl;
        }
        return action_();
    }
};

class StateMachine
{
private:
    State* current_state_;
    bool   terminated_ = false;

public:
    void SetInitialState(State& state)
    {
        current_state_ = &state;
    }

    void TakeAction()
    {
        if (!current_state_)
        {
            std::cerr << "No initial state - throw exception here" << std::endl;
        }

        if (!terminated_)
        {
            current_state_ = current_state_->TakeAction();
            terminated_    = current_state_ == nullptr;
        }
    }

    State* GetCurrentState()
    {
        return current_state_;
    }

    bool IsTerminated()
    {
        return terminated_;
    }
};

#endif
