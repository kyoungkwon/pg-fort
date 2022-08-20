#include <pqxx/pqxx>  // TODO: move

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

    // libpqxx
    pqxx::connection c{"postgresql://myusername:mypassword@postgresql:5432/mydatabase"};
    pqxx::work       w(c);

    // get table list
    // relkind
    //   r = ordinary table,
    //   i = index,
    //   S = sequence,
    //   t = TOAST table,
    //   v = view,
    //   m = materialized view,
    //   c = composite type,
    //   f = foreign table,
    //   p = partitioned table,
    //   I = partitioned index
    auto r = w.exec(
        "SELECT *"
        " FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace"
        " WHERE c.relkind = ANY(ARRAY['r','p','v','m','f'])"
        " AND pg_catalog.pg_table_is_visible(c.oid)"
        " AND n.nspname <> 'pg_catalog'"
        " AND n.nspname <> 'information_schema'"
        " AND n.nspname !~ '^pg_toast'");

    for (auto row : r)
    {
        // std::cout << "relname = " << row["relname"] << '\n';
        for (auto col : row)
        {
            std::cout << col.name() << " = " << col << '\n';
        }
        std::cout << "-------------------------------------------" << std::endl;
    }

    DbConnFactory factory(argv[2], argv[3]);

    ProxyServer proxy(proxy_port, factory);
    proxy.Run();

    std::cout << "exiting main.." << std::endl;
    
    return 0;
}
