#include <gtest/gtest.h>
#include <pg_query.h>

TEST(PgQueryTest, Simple)
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
        EXPECT_STREQ(expected[i], result.parse_tree);
        pg_query_free_parse_result(result);
    }

    // Optional, this ensures all memory is freed upon program exit (useful when running Valgrind)
    pg_query_exit();
}
