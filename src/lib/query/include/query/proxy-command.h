#ifndef __POSTGRESQL_PROXY_PROXYCOMMAND_H__
#define __POSTGRESQL_PROXY_PROXYCOMMAND_H__

#include <sstream>
#include <string>
#include <vector>

#include "common/error.h"
#include "query/query.h"

class ProxyCommand
{
private:
    bool valid_;

public:
    ProxyCommand();
    ProxyCommand(const ProxyCommand& c);
    ProxyCommand(ProxyCommand&& c) noexcept;
    ~ProxyCommand();

    ProxyCommand& operator=(const ProxyCommand& other);
    ProxyCommand& operator=(ProxyCommand&& other);
    explicit      operator bool() const;

    static std::pair<ProxyCommand, Error> Parse(const char* raw_command);

    char* ToString();
};

#endif
