#include "proxy-server/proxy-server.h"

ProxyServer::ProxyServer(int port, DbConnFactory *factory)
    : ip_("localhost"),
      port_(port),
      factory_(factory),
      flag_(true)
{
    // TODO: pass in a condition or something to handle shutdown
    pool_ = new SessionPool();
}

ProxyServer::~ProxyServer()
{
    // TODO: close and/or delete?
}

int ProxyServer::Initialize()
{
    int retval = 0;

    socket_                    = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);  // TODO: exception?
    sock_len_                  = sizeof(sock_addr_);
    sock_addr_.sin_family      = PF_INET;
    sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr_.sin_port        = htons(port_);

    retval = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &flag_, sizeof(flag_));
    BAIL_ON_ERROR(retval);

    retval = bind(socket_, (const sockaddr *)&sock_addr_, sock_len_);
    BAIL_ON_ERROR(retval);

    retval = fcntl(socket_, F_SETFL, O_NONBLOCK);
    BAIL_ON_ERROR(retval);

    retval = listen(socket_, 2048);
    BAIL_ON_ERROR(retval);

cleanup:
    return retval;

error:
    goto cleanup;
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

    static int loop = 0;

    while (true)
    {
        std::cout << "main loop (" << loop << ") starts" << std::endl;

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
        db_conn = factory_->CreateDbConn();

        std::cout << "dbconn initialize.." << std::endl;

        retval  = db_conn->Initialize();
        if (retval)
        {
            // TODO: better errno handling
            std::cerr << "dbconn initialize failed with retval = " << retval << std::endl;
            continue;
        }

        // create a session
        // session = new Session(newsocket, db_conn);
        Session session;

        // submit the session to the pool
        pool_->Submit(session);

        std::cout << "main loop (" << loop << ") ends" << std::endl;

        loop++;
    }

    std::cerr << "exiting proxy serve loop" << std::endl;
}
