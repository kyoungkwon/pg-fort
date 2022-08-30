#ifndef __POSTGRESQL_PROXY_REQUEST_H__
#define __POSTGRESQL_PROXY_REQUEST_H__

#include <cstring>

#include "conn/buffer.h"

class Request : public Buffer
{
public:
    void SetQuery(char* q)
    {
        auto qlen = strlen(q);
        auto mlen = qlen + 5;

        buf_.clear();
        buf_.push_back('Q');
        buf_.push_back(mlen >> 24);
        buf_.push_back(mlen >> 16);
        buf_.push_back(mlen >> 8);
        buf_.push_back(mlen);
        buf_.insert(buf_.end(), q, q + qlen + 1);

        data_size_ = buf_.size();
        buf_size_  = 0;
    }
};

#endif
