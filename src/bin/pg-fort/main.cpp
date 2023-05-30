#include "conn/pqxx-conn.h"
#include "proxy-server/proxy-server.h"
#include "schema/schema-tracker.h"

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

    auto db_host    = argv[2];
    auto db_port    = argv[3];
    auto proxy_port = atoi(argv[1]);

    auto scf = std::make_shared<ServerConnFactory>(db_host, db_port);
    auto pcp = std::make_shared<PqxxConnPool>(db_host, db_port, 10);
    auto st  = std::make_shared<SchemaTracker>(pcp);

    ProxyServer proxy(proxy_port, scf, pcp, st);
    proxy.Run();

    std::cout << "exiting main.." << std::endl;

    return 0;
}
