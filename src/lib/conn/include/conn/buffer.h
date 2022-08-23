#ifndef __POSTGRESQL_PROXY_BUFFER_H__
#define __POSTGRESQL_PROXY_BUFFER_H__

#include <sys/select.h>
#include <sys/socket.h>

#include <cstddef>
#include <vector>

class Buffer
{
protected:
    std::vector<char> buf_;
    std::size_t       buf_size_;
    std::size_t       data_size_;

public:
    Buffer();
    virtual ~Buffer();

    char*       Data();
    std::size_t Size();
    void        Reset();
    void        Take(std::vector<char>& data);

    int RecvFrom(int socket);
    int SendTo(int socket);
};

#endif
