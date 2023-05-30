#ifndef __PG_FORT_PROXYCOMMAND_H__
#define __PG_FORT_PROXYCOMMAND_H__

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
    static std::pair<std::string, Error> RemoveComments(std::string query);
    static std::pair<std::string, Error> TranslateEnableAccessControl(std::string query);
    static std::pair<std::string, Error> TranslateCreateAccessPermission(std::string query);
    static std::pair<std::string, Error> TranslateCreateAccessRole(std::string query);
    static std::pair<std::string, Error> TranslateCreateAccessInheritance(std::string query);
    static std::pair<std::string, Error> TranslateListAccessPermission(std::string query);
    static std::pair<std::string, Error> TranslateListAccessRole(std::string query);
    static std::pair<std::string, Error> TranslateListAccessInheritance(std::string query);
    static std::pair<std::string, Error> TranslateBindAccessRole(std::string query);
    static std::pair<std::string, Error> TranslateUnbindAccessRole(std::string query);
    static std::pair<std::string, Error> TranslateListAccessRoleBinding(std::string query);

    // TODO:
    static std::pair<std::string, Error> TranslateDeleteAccessRole(std::string query);
    static std::pair<std::string, Error> TranslateDisableAccessControl(std::string query);

public:
    static std::pair<std::string, Error> Translate(const char* query);
};

#endif
