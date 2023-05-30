#ifndef __PG_FORT_BUFFER_H__
#define __PG_FORT_BUFFER_H__

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

    int RecvFrom(int socket);
    int SendTo(int socket);
};

#endif
