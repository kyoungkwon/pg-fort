#include "proxy-server/proxy-server.h"

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <proxy-port> <db-host> <db-port>" << std::endl;
}

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        show_usage(argv[0]);
        exit(1);  // TODO: proper error code
    }

    auto proxy_port = atoi(argv[1]);

    DbConnFactory factory(argv[2], argv[3]);

    ProxyServer proxy(proxy_port, factory);
    proxy.Run();

    std::cout << "exiting main.." << std::endl;

    return 0;
}
