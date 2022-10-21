#include "query/proxy-command.h"

ProxyCommand::ProxyCommand()
    : valid_(false)
{
}

ProxyCommand::ProxyCommand(const ProxyCommand& c)
    : valid_(c.valid_)
{
}

ProxyCommand::ProxyCommand(ProxyCommand&& c) noexcept
    : valid_(std::move(c.valid_))
{
}

ProxyCommand::~ProxyCommand()
{
}

ProxyCommand& ProxyCommand::operator=(const ProxyCommand& other)
{
    valid_ = other.valid_;
    return *this;
}

ProxyCommand& ProxyCommand::operator=(ProxyCommand&& other)
{
    valid_ = std::move(other.valid_);
    return *this;
}

ProxyCommand::operator bool() const
{
    return valid_;
}

std::pair<ProxyCommand, Error> ProxyCommand::Parse(const char* raw_command)
{
    // TODO
    ProxyCommand p;
    return {std::move(p), NoError};
}

char* ProxyCommand::ToString()
{
    // TODO
    return nullptr;
}
