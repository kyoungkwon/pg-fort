#include "common/macros.h"
#include "conn/dbconn.h"
#include "proxy-server/proxy-server.h"

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <proxy-port> <db-port>" << std::endl;
}

int main(int argc, char const *argv[])
{
    int            retval    = 0;
    int            proxyPort = 0;
    int            dbPort    = 0;
    ProxyServer   *proxy     = NULL;
    DbConnFactory *factory   = NULL;

    if (argc != 3)
    {
        show_usage(argv[0]);
        BAIL_WITH_ERROR(retval, 1);  // TODO: proper error code
    }

    dbPort  = atoi(argv[2]);
    factory = new DbConnFactory(dbPort);

    proxyPort = atoi(argv[1]);
    proxy     = new ProxyServer(proxyPort, factory);

    retval = proxy->Initialize();
    BAIL_ON_ERROR(retval);

    proxy->Start();

    // TODO: better termination handling
    std::cout << "exiting.." << std::endl;

cleanup:
    delete proxy;
    delete factory;
    return retval;

error:
    goto cleanup;
}
