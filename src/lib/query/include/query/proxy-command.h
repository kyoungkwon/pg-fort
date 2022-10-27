#ifndef __POSTGRESQL_PROXY_PROXYCOMMAND_H__
#define __POSTGRESQL_PROXY_PROXYCOMMAND_H__

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "common/error.h"
#include "query/query.h"

class ProxyCommand
{
private:
    Query q_;

    static std::pair<std::string, Error> RemoveComments(std::string command);
    static std::pair<std::string, Error> ParseEnableAccessControl(std::string command);
    static std::pair<std::string, Error> ParseCreateAccessPermission(std::string command);
    static std::pair<std::string, Error> ParseCreateAccessRole(std::string command);
    static std::pair<std::string, Error> ParseCreateAccessInheritance(std::string command);
    static std::pair<std::string, Error> ParseListAccessPermission(std::string command);
    static std::pair<std::string, Error> ParseListAccessRole(std::string command);
    static std::pair<std::string, Error> ParseListAccessInheritance(std::string command);
    static std::pair<std::string, Error> ParseBindAccessRole(std::string command);
    static std::pair<std::string, Error> ParseUnbindAccessRole(std::string command);

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
