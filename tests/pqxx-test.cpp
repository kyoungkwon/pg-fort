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
        std::cout << "relname = " << row["relname"] << '\n';
        // for (auto col : row)
        // {
        //     std::cout << col.name() << " = " << col << '\n';
        // }
        std::cout << "-------------------------------------------" << std::endl;
    }
}

TEST(PqxxTest, PqxxConnPoolTest)
{
    PqxxConnPool cp("postgresql", "5432", 10);

    auto c0 = cp.Acquire();
    ASSERT_NE(c0, nullptr);

    std::cout << "c0 acquired" << std::endl;

    auto c1 = cp.Acquire();
    ASSERT_NE(c1, nullptr);

    std::cout << "c1 acquired" << std::endl;

    auto c2 = cp.Acquire();
    ASSERT_NE(c2, nullptr);

    std::cout << "c2 acquired" << std::endl;

    auto c3 = cp.Acquire();
    ASSERT_NE(c3, nullptr);

    std::cout << "c3 acquired" << std::endl;

    auto c4 = cp.Acquire();
    ASSERT_NE(c4, nullptr);

    std::cout << "c4 acquired" << std::endl;

    {
        auto c5 = cp.Acquire();
        ASSERT_NE(c5, nullptr);

        std::cout << "c5 acquired" << std::endl;

        auto c6 = cp.Acquire();
        ASSERT_NE(c6, nullptr);

        std::cout << "c6 acquired" << std::endl;

        auto c7 = cp.Acquire();
        ASSERT_NE(c7, nullptr);

        std::cout << "c7 acquired" << std::endl;

        auto c8 = cp.Acquire();
        ASSERT_NE(c8, nullptr);

        std::cout << "c8 acquired" << std::endl;

        auto c9 = cp.Acquire();
        ASSERT_NE(c9, nullptr);

        std::cout << "c9 acquired" << std::endl;

        ASSERT_EQ(cp.Acquire(100), nullptr);
        ASSERT_EQ(cp.Acquire(100), nullptr);
        ASSERT_EQ(cp.Acquire(100), nullptr);
    }

    auto c10 = cp.Acquire();
    ASSERT_NE(c10, nullptr);

    std::cout << "c10 acquired" << std::endl;

    auto c11 = cp.Acquire();
    ASSERT_NE(c11, nullptr);

    std::cout << "c11 acquired" << std::endl;

    auto c12 = cp.Acquire();
    ASSERT_NE(c12, nullptr);

    std::cout << "c12 acquired" << std::endl;

    auto c13 = cp.Acquire();
    ASSERT_NE(c13, nullptr);

    std::cout << "c13 acquired" << std::endl;

    auto c14 = cp.Acquire();
    ASSERT_NE(c14, nullptr);

    std::cout << "c14 acquired" << std::endl;

    ASSERT_EQ(cp.Acquire(100), nullptr);
    ASSERT_EQ(cp.Acquire(100), nullptr);
    ASSERT_EQ(cp.Acquire(100), nullptr);

    pqxx::work w(*c14);

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
        std::cout << "relname = " << row["relname"] << '\n';
        std::cout << "-------------------------------------------" << std::endl;
    }
}
