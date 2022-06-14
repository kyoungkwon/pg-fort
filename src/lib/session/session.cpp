#include "session/session.h"

Session::Session(int socket, DbConn *dbConn)
    : socket_(socket),
      dbConn_(dbConn)
{
    // auto flags = fcntl(socket_, F_GETFL, 0);

    // if (flags < 0)
    // {
    //     std::cerr << std::this_thread::get_id() << ": can't get flags";
    // }

    // flags &= ~O_NONBLOCK;

    // if (fcntl(socket_, F_SETFL, flags) < 0)
    // {
    //     std::cerr << std::this_thread::get_id() << ": can't get flags";
    // }
}

Session::~Session()
{
    // TODO: close and/or delete?
}

void Session::Serve(/*, plugins? */)
{
    try
    {
        while (true)
        {
            // // while loop inside ReceiveRequest
            // auto req = clConn_->ReceiveRequest(retry=?);  // throws exception e.g., connection lost

            // // plugins?

            // // send inside ForwardRequest
            // dbConn_->ForwardRequest(req, retry=?);  // throws exception e.g., connection lost

            // // plugins?

            // // while loop inside ReceiveResponse
            // auto resp = dbConn_->ReceiveResponse(retry=?);  // throws exception e.g., connection lost

            // // plugins?

            // // send inside ForwardResponse
            // clConn_->ForwardResponse(resp, retry=?);  // throws exception e.g., connection lost
        }
    }
    catch (...)
    {
        std::cerr << "something bad happened" << std::endl;
    }
    // TODO: crashes when cli disconnect - find out why and fix it
    // TODO: cli hangs when return key was entered when the line doesn't end with ;
}

void Session::Serve2()
{
    std::cout << std::this_thread::get_id() << ": session starts" << std::endl;

    while (true)
    {
        std::cout << std::this_thread::get_id() << ": transmit 1" << std::endl;

        // transmit request from client to db
        Transmit3(socket_, dbConn_->GetSocket());

        std::cout << std::this_thread::get_id() << ": transmit 2" << std::endl;

        // transmit response from db to client
        Transmit3(dbConn_->GetSocket(), socket_);

        std::cout << std::this_thread::get_id() << ": transmit 3" << std::endl;
    }

    std::cout << std::this_thread::get_id() << ": session ends" << std::endl;

    // OR

    /*
    // run two threads
    //  - (1) reading from client and writing to db
    //  - (2) reading from db and writing to client
    // two threads will continue to run until either connection is broken

    // TODO: pass stop_tokens to stop them upon shutdown

    // threads need a shared variable to communicate failure

    std::stop_source ssrc;

    std::jthread forward(Transmit, socket_, dbConn_->GetSocket(), ssrc);
    std::jthread reverse(Transmit, dbConn_->GetSocket(), socket_, ssrc);

    forward.join();
    reverse.join();
    */
}

void Session::Transmit2(int from_socket, int to_socket)
{
    int                        retval = 0;
    fd_set                     fds;
    struct timeval             timeout  = {0};
    std::size_t                buf_size = 1024;
    std::vector<unsigned char> buf(buf_size * 2);
    int                        bytes_total = 0;

    // set select timeout to 3 sec
    timeout.tv_sec  = 3;
    timeout.tv_usec = 0;

    // read from_socket in a loop
    while (true)
    {
        FD_ZERO(&fds);
        FD_SET(from_socket, &fds);
        retval = select(from_socket + 1, &fds, NULL, NULL, &timeout);
        if (retval == 0)
        {
            std::cout << std::this_thread::get_id() << ": recv select timeout = " << retval
                      << std::endl;
            break;
        }
        else if (retval < 0)
        {
            std::cout << std::this_thread::get_id() << ": recv select failed = " << retval
                      << std::endl;
            return;
        }

        retval = recv(from_socket, buf.data() + bytes_total, buf_size, 0);
        std::cout << std::this_thread::get_id() << ": recv retval = " << retval << std::endl;

        if (retval > 0)
        {
            bytes_total += retval;
            if (bytes_total > buf_size)
            {
                // double buffer size when half-full
                buf_size *= 2;
                buf.resize(buf_size * 2);
            }
            continue;
        }

        if (retval < 0)
        {
            std::cerr << std::this_thread::get_id() << ": recv errno = " << errno << std::endl;
        }

        break;
    }

    std::cout << std::this_thread::get_id() << ": recv total " << bytes_total << std::endl;

    // write to_socket

    FD_ZERO(&fds);
    FD_SET(to_socket, &fds);
    retval = select(to_socket + 1, &fds, NULL, NULL, &timeout);
    if (retval == 0)
    {
        std::cout << std::this_thread::get_id() << ": send select timeout = " << retval
                  << std::endl;
        return;
    }
    else if (retval < 0)
    {
        std::cout << std::this_thread::get_id() << ": send select failed = " << retval << std::endl;
        return;
    }

    retval = send(to_socket, buf.data(), bytes_total, 0);
    std::cout << std::this_thread::get_id() << ": send retval = " << retval << std::endl;

    if (retval < 0)
    {
        std::cerr << std::this_thread::get_id() << ": send errno = " << errno << std::endl;
    }

    BAIL_ON_ERROR_IF(retval < 0);  // TODO: pass errno

    std::cout << std::this_thread::get_id() << ": send total " << retval << std::endl;

cleanup:
    return;

error:
    // TODO: implement thread context and log error info with function name and line
    std::cerr << "Transmit failed" << std::endl;
    goto cleanup;
}

void Session::Transmit3(int from_socket, int to_socket)
{
    int                        retval = 0;
    fd_set                     fds;
    struct timeval             timeout  = {0};
    std::size_t                buf_size = 1024;
    std::vector<unsigned char> buf(buf_size * 2);
    int                        bytes_total = 0;

    // set select timeout to 3 sec
    timeout.tv_sec  = 3;
    timeout.tv_usec = 0;

    // read from_socket
    FD_ZERO(&fds);
    FD_SET(from_socket, &fds);
    retval = select(from_socket + 1, &fds, NULL, NULL, &timeout);
    if (retval == 0)
    {
        std::cout << std::this_thread::get_id() << ": recv select timeout = " << retval
                  << std::endl;
        return;
    }
    else if (retval < 0)
    {
        std::cout << std::this_thread::get_id() << ": recv select failed = " << retval << std::endl;
        return;
    }

    retval = recv(from_socket, buf.data() + bytes_total, buf_size, 0);
    std::cout << std::this_thread::get_id() << ": recv retval = " << retval << std::endl;

    if (retval < 0)
    {
        std::cerr << std::this_thread::get_id() << ": recv errno = " << errno << std::endl;
    }

    bytes_total = retval;
    std::cout << std::this_thread::get_id() << ": recv total " << bytes_total << std::endl;

    // write to_socket
    FD_ZERO(&fds);
    FD_SET(to_socket, &fds);
    retval = select(to_socket + 1, NULL, &fds, NULL, &timeout);
    if (retval == 0)
    {
        std::cout << std::this_thread::get_id() << ": send select timeout = " << retval
                  << std::endl;
        return;
    }
    else if (retval < 0)
    {
        std::cout << std::this_thread::get_id() << ": send select failed = " << retval << std::endl;
        return;
    }

    retval = send(to_socket, buf.data(), bytes_total, 0);
    std::cout << std::this_thread::get_id() << ": send retval = " << retval << std::endl;

    if (retval < 0)
    {
        std::cerr << std::this_thread::get_id() << ": send errno = " << errno << std::endl;
    }

    std::cout << std::this_thread::get_id() << ": send total " << retval << std::endl;

cleanup:
    return;

error:
    // TODO: implement thread context and log error info with function name and line
    std::cerr << "Transmit failed" << std::endl;
    goto cleanup;
}

SessionPool::SessionPool()
{
    // TODO: create a worker pool + session queue
}

SessionPool::~SessionPool()
{
}

// TODO: maybe we need session handler initialize and start as well

int SessionPool::Submit(Session *session)
{
    // TODO push_back to the session queue
    std::thread thr(&Session::Serve, session);
    thr.detach();
    return 0;
}
