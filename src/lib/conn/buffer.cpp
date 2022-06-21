#include "conn/buffer.h"

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

unsigned char* Buffer::Data()
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
    // set select timeout to 3 sec
    struct timeval timeout = {0};
    timeout.tv_sec         = 3;
    timeout.tv_usec        = 0;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket, &fds);

    int retval = select(socket + 1, &fds, NULL, NULL, &timeout);
    if (retval <= 0)
    {
        return retval;  // TODO: better handling
    }

    while (true)
    {
        retval = recv(socket, buf_.data() + data_size_, buf_size_, 0);
        if (retval < 0)
        {
            return errno == EAGAIN ? data_size_ : retval;  // TODO: better handling
        }
        else if (retval == 0)
        {
            return data_size_;  // complete
        }

        data_size_ += retval;
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
    int retval;
    fd_set      fds;

    // set select timeout to 3 sec
    struct timeval timeout = {0};
    timeout.tv_sec         = 3;
    timeout.tv_usec        = 0;

    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    retval = select(socket + 1, NULL, &fds, NULL, &timeout);
    if (retval <= 0)
    {
        return retval;  // TODO: better handling
    }

    retval = send(socket, buf_.data(), data_size_, 0);
    return retval;
}
