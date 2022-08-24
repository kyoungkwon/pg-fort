#include "proxy-server/proxy-server.h"

ProxyServer::ProxyServer(int port, std::shared_ptr<DbConnFactory> dbcf,
                         std::shared_ptr<SchemaTracker> st)
    : ip_("localhost"),
      port_(port),
      flag_(true),
      dbcf_(dbcf),
      st_(st),
      so_(4)
{
    int res = 0;

    sock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ < 0)
    {
        // TODO: throw exception
        std::cerr << "ProxyServer() socket failed: " << sock_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    sock_len_                  = sizeof(sock_addr_);
    sock_addr_.sin_family      = PF_INET;
    sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr_.sin_port        = htons(port_);

    res = setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &flag_, sizeof(flag_));
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "ProxyServer() setsockopt failed: " << sock_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    res = bind(sock_, (const sockaddr *)&sock_addr_, sock_len_);
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "ProxyServer() bind failed: " << sock_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    res = fcntl(sock_, F_SETFL, O_NONBLOCK);
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "ProxyServer() fcntl failed: " << sock_ << " (errno=" << errno << ")"
                  << std::endl;
    }

    res = listen(sock_, 2048);
    if (res < 0)
    {
        // TODO: throw exception
        std::cerr << "ProxyServer() listen failed: " << sock_ << " (errno=" << errno << ")"
                  << std::endl;
    }
}

ProxyServer::~ProxyServer()
{
    so_.Stop();
}

void ProxyServer::Run()
{
    fd_set fds;

    // set select timeout to 3 sec
    struct timeval timeout = {0};
    timeout.tv_sec         = 3;
    timeout.tv_usec        = 0;

    // start session operator
    so_.Start();

    while (true)
    {
        FD_ZERO(&fds);
        FD_SET(sock_, &fds);
        int res = select(sock_ + 1, &fds, NULL, NULL, &timeout);
        if (res == 0)
        {
            continue;  // timeout
        }
        else if (res < 0)
        {
            // TODO: better error logging
            std::cerr << "select failed with res = " << res << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // accept and create session
        int new_sock = accept(sock_, 0, 0);
        if (new_sock < 0)
        {
            // TODO: better errno handling
            std::cerr << "accept failed with errno = " << errno << std::endl;
            continue;
        }

        // create a client connection
        auto cl_conn = new ClientConn(new_sock);

        // create a db connection
        auto db_conn = dbcf_->CreateDbConn();

        // create a session and submit to the operator
        auto s = new Session(cl_conn, db_conn, st_);

        std::cout << "New session [" << s->id << "]" << std::endl;

        so_.Submit(s);
    }
}
