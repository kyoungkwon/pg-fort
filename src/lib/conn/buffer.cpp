#include "conn/buffer.h"

#include <iostream>

#define DEFAULT_SIZE 1024

Buffer::Buffer()
    : buf_(),
      buf_size_(0),
      data_size_(0)
{
}

Buffer::~Buffer()
{
    buf_.clear();
}

char* Buffer::Data()
{
    return buf_.data();
}

std::size_t Buffer::Size()
{
    return data_size_;
}

void Buffer::Reset()
{
    buf_.clear();
    buf_size_ = DEFAULT_SIZE;
    buf_.resize(2 * buf_size_);
    data_size_ = 0;
}

int Buffer::RecvFrom(int socket)
{
    while (true)
    {
        int res = recv(socket, buf_.data() + data_size_, buf_size_, 0);
        if (res < 0)
        {
            std::cout << "\t\033[43m"
                      << "res = " << res;
            std::cout << ", errno = " << errno;
            std::cout << ", data size = " << data_size_;
            std::cout << ", buf size = " << buf_.size();
            std::cout << "\033[0m";
            return errno == EAGAIN ? data_size_ : res;  // TODO: better handling
        }
        else if (res == 0)
        {
            std::cout << "\t\033[43m"
                      << "res = " << res;
            std::cout << ", data size = " << data_size_;
            std::cout << ", buf size = " << buf_.size();
            std::cout << "\033[0m";
            return data_size_;  // complete
        }

        data_size_ += res;
        if (data_size_ > buf_size_)
        {
            // double buffer size when half-full
            buf_size_ *= 2;
            buf_.resize(buf_size_ * 2);
        }
    }
}

int Buffer::SendTo(int socket)
{
    // TODO: add a loop - until retval += send(...) == data_size <-- just short-expr, not going to
    // work with retval < 0
    int res = send(socket, buf_.data(), data_size_, 0);
    return res;
}
