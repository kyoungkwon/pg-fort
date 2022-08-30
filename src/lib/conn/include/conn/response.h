#ifndef __POSTGRESQL_PROXY_RESPONSE_H__
#define __POSTGRESQL_PROXY_RESPONSE_H__

#include <cstring>

#include "conn/buffer.h"

class Response : public Buffer
{
public:
    void SetError(std::exception& e)
    {
        auto w    = e.what();
        auto wlen = strlen(w);
        auto mlen = 5 + 7 + 7 + 7 + wlen + 2;

        // TODO: these should be extracted from exception
        auto s = "ERROR";
        auto c = "42P16";

        // ErrorResponse
        buf_.clear();
        buf_.push_back('E');
        buf_.push_back(mlen >> 24);
        buf_.push_back(mlen >> 16);
        buf_.push_back(mlen >> 8);
        buf_.push_back(mlen);

        // S
        buf_.push_back('S');
        buf_.insert(buf_.end(), s, s + 6);

        // V
        buf_.push_back('V');
        buf_.insert(buf_.end(), s, s + 6);

        // C
        buf_.push_back('C');
        buf_.insert(buf_.end(), c, c + 6);

        // P? F? L? R?

        // M
        buf_.push_back('M');
        buf_.insert(buf_.end(), w, w + wlen + 1);
        buf_.push_back(0);

        // ReadyForQuery
        buf_.push_back('Z');
        buf_.push_back(0);
        buf_.push_back(0);
        buf_.push_back(0);
        buf_.push_back(5);
        buf_.push_back('I');  // TODO: reflect txn status properly

        data_size_ = buf_.size();
        buf_size_  = 0;
    }
};

#endif
