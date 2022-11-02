#ifndef __POSTGRESQL_PROXY_JSONUTIL_H__
#define __POSTGRESQL_PROXY_JSONUTIL_H__

#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

class JsonUtil
{
public:
    static json* FindNode(json& j, std::string key)
    {
        for (auto& [k, v] : j.items())
        {
            if (k == key)
            {
                return &v;
            }
            else if (!v.is_primitive())
            {
                if (auto n = FindNode(v, key); n != nullptr)
                {
                    return n;
                }
            }
        }
        return nullptr;
    }
};

#endif
