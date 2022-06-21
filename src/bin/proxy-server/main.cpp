#include "conn/db-conn.h"
#include "proxy-server/proxy-server.h"

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <proxy-port> <db-port>" << std::endl;
}

int main(int argc, char const *argv[])
{
    int retval    = 0;
    int proxyPort = 0;
    int dbPort    = 0;

    if (argc != 3)
    {
        show_usage(argv[0]);
        exit(1);  // TODO: proper error code
    }

    dbPort = atoi(argv[2]);
    DbConnFactory factory("127.0.0.1", dbPort);

    proxyPort = atoi(argv[1]);
    ProxyServer proxy(proxyPort, factory);
    proxy.Start();

    std::cout << "exiting main.." << std::endl;

    return retval;
}
