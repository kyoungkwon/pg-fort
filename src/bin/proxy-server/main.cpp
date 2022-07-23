#include "proxy-server/proxy-server.h"

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <proxy-port> <db-port>" << std::endl;
}

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        show_usage(argv[0]);
        exit(1);  // TODO: proper error code
    }

    auto          db_host = std::string(argv[2]);
    auto          db_port = atoi(argv[3]);
    DbConnFactory factory(db_host, db_port);

    auto        proxy_port = atoi(argv[1]);
    ProxyServer proxy(proxy_port, factory);
    proxy.Run();

    std::cout << "exiting main.." << std::endl;

    return 0;
}
