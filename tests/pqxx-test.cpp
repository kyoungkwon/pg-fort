#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>

#include <pqxx/pqxx>
#include <sstream>

#include "conn/pqxx-conn.h"

TEST(PqxxTest, Constructor)
{
    PqxxConn   c("postgresql", "5432");
    pqxx::work w(c);

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
}
