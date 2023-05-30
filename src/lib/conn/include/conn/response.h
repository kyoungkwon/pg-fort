#ifndef __PG_FORT_RESPONSE_H__
#define __PG_FORT_RESPONSE_H__

#include <assert.h>

#include <cstring>
#include <map>

#include "common/error.h"
#include "conn/buffer.h"

class Response : public Buffer
{
public:
    // reference: https://www.postgresql.org/docs/current/protocol-error-fields.html
    void SetError(const char* s, const char* c, const char* m, const char* f, const char* r, const char* l,
                  const char* p)
    {
        uint32_t total_len = 5;
        std::cout << "total_len = " << total_len << std::endl;

        // reinitialize buffer
        buf_.clear();
        buf_.push_back('E');
        buf_.push_back(0);
        buf_.push_back(0);
        buf_.push_back(0);
        buf_.push_back(0);

        // S and V are identical
        const char* v = s;

        // always present fields
        std::map<const char, const char*> mandatory_fields = {
            {'S', s},
            {'V', v},
            {'C', c},
            {'M', m}
        };

        for (auto& [key, val] : mandatory_fields)
        {
            assert(key && val);
            auto len = strlen(val);
            buf_.push_back(key);
            buf_.insert(buf_.end(), val, val + len + 1);
            total_len += len + 2;
            std::cout << "total_len = " << total_len << std::endl;
        }

        // optional fields
        std::map<const char, const char*> optional_fields = {
            {'F', f},
            {'R', r},
            {'L', l},
            {'P', p}
        };

        for (auto& [key, val] : optional_fields)
        {
            if (val != nullptr)
            {
                auto len = strlen(val);
                buf_.push_back(key);
                buf_.insert(buf_.end(), val, val + len + 1);
                total_len += len + 2;
                std::cout << "total_len = " << total_len << std::endl;
            }
        }

        // eom indicator
        buf_.push_back(0);

        // update the total size
        buf_[1] = total_len >> 24;
        buf_[2] = total_len >> 16;
        buf_[3] = total_len >> 8;
        buf_[4] = total_len;

        // ReadyForQuery
        buf_.push_back('Z');
        buf_.push_back(0);
        buf_.push_back(0);
        buf_.push_back(0);
        buf_.push_back(5);
        buf_.push_back('I');  // TODO: reflect txn status properly

        // set size vars
        data_size_ = buf_.size();
        buf_size_  = 0;
    }

    void SetError(Error err)
    {
        SetError(err.contains("S") ? err["S"].c_str() : nullptr, err.contains("C") ? err["C"].c_str() : nullptr,
                 err.contains("M") ? err["M"].c_str() : nullptr, err.contains("F") ? err["F"].c_str() : nullptr,
                 err.contains("R") ? err["R"].c_str() : nullptr, err.contains("L") ? err["L"].c_str() : nullptr,
                 err.contains("P") ? err["P"].c_str() : nullptr);
    }
};

#endif
