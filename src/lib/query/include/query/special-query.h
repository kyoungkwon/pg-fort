#ifndef __POSTGRESQL_PROXY_SPECIALQUERY_H__
#define __POSTGRESQL_PROXY_SPEICALQUERY_H__

#include <sstream>
#include <string>
#include <vector>

class SpecialQuery
{
private:
	// 0: command = { CREATE, ALTER, DELETE }
	// 1: ACCESS
	// 2: type = { PERMISSION, ROLE, INHERITANCE, BINDING }
	// 3: name
	// 4: ON?
    std::vector<std::string> tokens_;

public:
    SpecialQuery(const char* raw_query);
    ~SpecialQuery();

    char* Translate();
    // Query Translate();
};

#endif
