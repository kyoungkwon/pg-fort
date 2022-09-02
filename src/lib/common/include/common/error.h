#ifndef __POSTGRESQL_PROXY_ERROR_H__
#define __POSTGRESQL_PROXY_ERROR_H__

#include <string>
#include <unordered_map>

class Error
{
protected:
    std::unordered_map<std::string, std::string> m_;

public:
    Error()
    {
    }

    Error(std::initializer_list<std::pair<const std::string, std::string>> init)
        : m_(init)
    {
    }

    explicit operator bool() const
    {
        return !m_.empty();
    };

    std::unordered_map<std::string, std::string>::iterator begin()
    {
        return m_.begin();
    }

    std::unordered_map<std::string, std::string>::iterator end()
    {
        return m_.end();
    }

    std::unordered_map<std::string, std::string>::const_iterator begin() const
    {
        return m_.begin();
    }

    std::unordered_map<std::string, std::string>::const_iterator end() const
    {
        return m_.end();
    }

    std::string& operator[](const std::string& key)
    {
        return m_[key];
    }

    std::string& operator[](std::string&& key)
    {
        return m_[key];
    }

    bool contains(const std::string& key) const
    {
        return m_.contains(key);
    }
};

const static Error NoError;

#endif
