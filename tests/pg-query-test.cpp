#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>
#include <pg_query.h>
#include <pg_query/pg_query.pb-c.h>
#include <pg_query.pb.h>

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST(PgQueryTest, Parse)
{
    // clang-format off
    const char* test_cases[] = {
        "SELECT 1",
        "SELECT * FROM x WHERE z = 2",
        "SELECT 5.41414",
        "SELECT $1",
        "SELECT ?",
        "SELECT 999999999999999999999::numeric/1000000000000000000000",
        "SELECT 4790999999999999999999999999999999999999999999999999999999999999999999999999999999999999 * 9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
    };

    const char* expected[] = {
        "{\"version\":130003,\"stmts\":[{\"stmt\":{\"SelectStmt\":{\"targetList\":[{\"ResTarget\":{\"val\":{\"A_Const\":{\"val\":{\"Integer\":{\"ival\":1}},\"location\":7}},\"location\":7}}],\"limitOption\":\"LIMIT_OPTION_DEFAULT\",\"op\":\"SETOP_NONE\"}}}]}",
        "{\"version\":130003,\"stmts\":[{\"stmt\":{\"SelectStmt\":{\"targetList\":[{\"ResTarget\":{\"val\":{\"ColumnRef\":{\"fields\":[{\"A_Star\":{}}],\"location\":7}},\"location\":7}}],\"fromClause\":[{\"RangeVar\":{\"relname\":\"x\",\"inh\":true,\"relpersistence\":\"p\",\"location\":14}}],\"whereClause\":{\"A_Expr\":{\"kind\":\"AEXPR_OP\",\"name\":[{\"String\":{\"str\":\"=\"}}],\"lexpr\":{\"ColumnRef\":{\"fields\":[{\"String\":{\"str\":\"z\"}}],\"location\":22}},\"rexpr\":{\"A_Const\":{\"val\":{\"Integer\":{\"ival\":2}},\"location\":26}},\"location\":24}},\"limitOption\":\"LIMIT_OPTION_DEFAULT\",\"op\":\"SETOP_NONE\"}}}]}",
        "{\"version\":130003,\"stmts\":[{\"stmt\":{\"SelectStmt\":{\"targetList\":[{\"ResTarget\":{\"val\":{\"A_Const\":{\"val\":{\"Float\":{\"str\":\"5.41414\"}},\"location\":7}},\"location\":7}}],\"limitOption\":\"LIMIT_OPTION_DEFAULT\",\"op\":\"SETOP_NONE\"}}}]}",
        "{\"version\":130003,\"stmts\":[{\"stmt\":{\"SelectStmt\":{\"targetList\":[{\"ResTarget\":{\"val\":{\"ParamRef\":{\"number\":1,\"location\":7}},\"location\":7}}],\"limitOption\":\"LIMIT_OPTION_DEFAULT\",\"op\":\"SETOP_NONE\"}}}]}",
        "{\"version\":130003,\"stmts\":[{\"stmt\":{\"SelectStmt\":{\"targetList\":[{\"ResTarget\":{\"val\":{\"ParamRef\":{\"location\":7}},\"location\":7}}],\"limitOption\":\"LIMIT_OPTION_DEFAULT\",\"op\":\"SETOP_NONE\"}}}]}",
        "{\"version\":130003,\"stmts\":[{\"stmt\":{\"SelectStmt\":{\"targetList\":[{\"ResTarget\":{\"val\":{\"A_Expr\":{\"kind\":\"AEXPR_OP\",\"name\":[{\"String\":{\"str\":\"/\"}}],\"lexpr\":{\"TypeCast\":{\"arg\":{\"A_Const\":{\"val\":{\"Float\":{\"str\":\"999999999999999999999\"}},\"location\":7}},\"typeName\":{\"names\":[{\"String\":{\"str\":\"pg_catalog\"}},{\"String\":{\"str\":\"numeric\"}}],\"typemod\":-1,\"location\":30},\"location\":28}},\"rexpr\":{\"A_Const\":{\"val\":{\"Float\":{\"str\":\"1000000000000000000000\"}},\"location\":38}},\"location\":37}},\"location\":7}}],\"limitOption\":\"LIMIT_OPTION_DEFAULT\",\"op\":\"SETOP_NONE\"}}}]}",
        "{\"version\":130003,\"stmts\":[{\"stmt\":{\"SelectStmt\":{\"targetList\":[{\"ResTarget\":{\"val\":{\"A_Expr\":{\"kind\":\"AEXPR_OP\",\"name\":[{\"String\":{\"str\":\"*\"}}],\"lexpr\":{\"A_Const\":{\"val\":{\"Float\":{\"str\":\"4790999999999999999999999999999999999999999999999999999999999999999999999999999999999999\"}},\"location\":7}},\"rexpr\":{\"A_Const\":{\"val\":{\"Float\":{\"str\":\"9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\"}},\"location\":98}},\"location\":96}},\"location\":7}}],\"limitOption\":\"LIMIT_OPTION_DEFAULT\",\"op\":\"SETOP_NONE\"}}}]}"
    };
    // clang-format on

    auto n = sizeof(test_cases) / sizeof(test_cases[0]);
    ASSERT_EQ(sizeof(expected) / sizeof(expected[0]), n);

    PgQueryParseResult result;
    for (int i = 0; i < n; i++)
    {
        result = pg_query_parse(test_cases[i]);
        EXPECT_EQ(result.error, nullptr);
        EXPECT_STREQ(result.parse_tree, expected[i]);
        pg_query_free_parse_result(result);
    }
}

TEST(PgQueryTest, ParseDeparse)
{
    // clang-format off
    const char* test_cases[] = {
        "SELECT 1",
        "SELECT * FROM x WHERE z = 2",
        "SELECT 5.41414",
        "SELECT $1",
        "SELECT ?",
        "SELECT 999999999999999999999::numeric/1000000000000000000000",
        "SELECT 4790999999999999999999999999999999999999999999999999999999999999999999999999999999999999 * 9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
    };
    // clang-format on

    auto n = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < n; i++)
    {
        PgQueryProtobufParseResult result = pg_query_parse_protobuf(test_cases[i]);
        EXPECT_EQ(result.error, nullptr);

        PgQueryDeparseResult deparse_result = pg_query_deparse_protobuf(result.parse_tree);
        EXPECT_EQ(deparse_result.error, nullptr);

        // EXPECT_STREQ(deparse_result.query, test_cases[i]);

        pg_query_free_deparse_result(deparse_result);
        pg_query_free_protobuf_parse_result(result);
    }
}

TEST(PgQueryTest, ParseUnpack)
{
    // clang-format off
    const char* test_cases[] = {
        "SELECT 1",
        "SELECT * FROM x WHERE z = 2",
        "SELECT 5.41414",
        "SELECT $1",
        "SELECT ?",
        "SELECT 999999999999999999999::numeric/1000000000000000000000",
        "SELECT 4790999999999999999999999999999999999999999999999999999999999999999999999999999999999999 * 9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
    };
    // clang-format on

    auto n = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < n; i++)
    {
        PgQueryProtobufParseResult result = pg_query_parse_protobuf(test_cases[i]);
        EXPECT_EQ(result.error, nullptr);

        PgQueryProtobuf       pbuf = result.parse_tree;
        PgQuery__ParseResult* parse_result =
            pg_query__parse_result__unpack(NULL, pbuf.len, (const uint8_t*)pbuf.data);

        pg_query__parse_result__free_unpacked(parse_result, NULL);
        pg_query_free_protobuf_parse_result(result);
    }
}

TEST(PgQueryTest, ParseUnpackEditDeparse)
{
    // clang-format off
    const char* test_cases[] = {
        // no change
        "SELECT 1",

        // simple case (before)
        "SELECT * FROM x WHERE z = 2",

        // simple case (after)
        "SELECT x.* FROM x JOIN x_acl ON x.id = x_acl.x_id AND x_acl.perm_id = 1 AND x_acl.principal = 'kkwon' WHERE x.z = 2",

        // more complex cases
        /*
        "SELECT DISTINCT ON (location) location, time, report FROM weather_reports ORDER BY location, time DESC;",
        "SELECT * FROM (SELECT * FROM mytable FOR UPDATE) ss WHERE col1 = 5;",
        "SELECT f.title, f.did, d.name, f.date_prod, f.kind FROM distributors d, films f WHERE f.did = d.did",
        "SELECT kind, sum(len) AS total FROM films GROUP BY kind;",
        "SELECT kind, sum(len) AS total FROM films GROUP BY kind HAVING sum(len) < interval '5 hours';",
        "SELECT distributors.name FROM distributors WHERE distributors.name LIKE 'W%' UNION SELECT actors.name FROM actors WHERE actors.name LIKE 'W%';",

        // TODO: experiment if WITH can reuse an existing table name
        "WITH t AS (SELECT random() as x FROM generate_series(1, 3)) SELECT * FROM t UNION ALL SELECT * FROM t",

        "WITH RECURSIVE employee_recursive(distance, employee_name, manager_name) AS (SELECT 1, employee_name, manager_name FROM employee WHERE manager_name = 'Mary' UNION ALL SELECT er.distance + 1, e.employee_name, e.manager_name FROM employee_recursive er, employee e WHERE er.employee_name = e.manager_name) SELECT distance, employee_name FROM employee_recursive;",
        "SELECT m.name AS mname, pname FROM manufacturers m, LATERAL get_product_names(m.id) pname;",
        "SELECT m.name AS mname, pname FROM manufacturers m LEFT JOIN LATERAL get_product_names(m.id) pname ON true;"
        */
    };
    // clang-format on

    auto n = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < n; i++)
    {
        // output file
        char f[100] = {0};
        sprintf(f, "/workspace/tests/out/parsed_%02d.json", i);
        std::ofstream o(f);
        std::cout << "out: " << f << std::endl;

        // raw query string
        std::cout << " before: " << test_cases[i] << std::endl;

        // parse query
        auto parse_result = pg_query_parse(test_cases[i]);
        EXPECT_EQ(parse_result.error, nullptr);

        // json
        auto j = json::parse(parse_result.parse_tree);

        // pretty printing
        o << j.dump(4) << std::endl;

        // TODO: mod query - by recursion?

        // mod result
        pg_query::ParseResult mod_result;
        google::protobuf::util::JsonStringToMessage(j.dump(), &mod_result);

        // serialize mod result
        std::string output;
        mod_result.SerializeToString(&output);

        // copy to pbuf
        PgQueryProtobuf pbuf;
        pbuf.data = (char*)calloc(output.size(), sizeof(char));  // TODO: free
        memcpy(pbuf.data, output.data(), output.size());
        pbuf.len = output.size();

        // deparse into query string
        PgQueryDeparseResult deparse_result = pg_query_deparse_protobuf(pbuf);
        EXPECT_EQ(deparse_result.error, nullptr);

        // modded query string
        std::cout << " after:  " << deparse_result.query << std::endl;
        std::cout << "-----------------------------------------------------" << std::endl;

        pg_query_free_deparse_result(deparse_result);
        pg_query_free_parse_result(parse_result);
        o.close();
    }
}
