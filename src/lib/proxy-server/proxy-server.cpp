#include "proxy-server/proxy-server.h"

ProxyServer::ProxyServer(int port, DbConnFactory &factory)
    : ip_("localhost"),
      port_(port),
      factory_(factory),
      flag_(true)
{
    int res = 0;

    socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ < 0)
    {
        // TODO: throw exception
        std::cerr << "ProxyServer() socket failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    sock_len_                  = sizeof(sock_addr_);
    sock_addr_.sin_family      = PF_INET;
    sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr_.sin_port        = htons(port_);

    res = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &flag_, sizeof(flag_));
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() setsockopt failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    res = bind(socket_, (const sockaddr *)&sock_addr_, sock_len_);
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() bind failed: " << socket_ << " (errno=" << errno << ")" << std::endl;
    }

    res = fcntl(socket_, F_SETFL, O_NONBLOCK);
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() fcntl failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    res = listen(socket_, 2048);
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "DbConn() listen failed: " << socket_ << " (errno=" << errno << ")"
                  << std::endl;
    }
}

ProxyServer::~ProxyServer()
{
    pool_.Stop();
}

void ProxyServer::Start()
{
    int            retval = 0;
    fd_set         fds;
    struct timeval timeout = {0};
    int            newsocket;
    DbConn        *db_conn = NULL;
    Session       *session = NULL;

    // set select timeout to 3 sec
    timeout.tv_sec  = 3;
    timeout.tv_usec = 0;

    while (true)
    {
        FD_ZERO(&fds);
        FD_SET(socket_, &fds);
        retval = select(socket_ + 1, &fds, NULL, NULL, &timeout);
        if (retval == 0)
        {
            // timed out
            continue;
        }
        else if (retval < 0)
        {
            // TODO: better error logging
            std::cerr << "select failed with retval = " << retval << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // accept and create session
        newsocket = accept(socket_, 0, 0);
        if (newsocket < 0)
        {
            // TODO: better errno handling
            std::cerr << "accept failed with errno = " << errno << std::endl;
            continue;
        }

        // create a db connection
        db_conn = factory_.CreateDbConn();

        // create a session and submit to the pool
        // session = new Session(newsocket, db_conn);
        Session session;
        pool_.Submit(session);
    }
}
