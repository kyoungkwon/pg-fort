#include "query/query.h"

#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>
#include <pg_query.h>
#include <pg_query/pg_query.pb-c.h>

#include <chrono>
#include <fstream>
#include <vector>

#include "query/jsonutil.h"
#include "query/query-acler.h"
#include "schema/schema-tracker.h"

using json = nlohmann::json;

TEST(QueryTest, Parse)
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

        // copy
        auto c = result;
        EXPECT_EQ(c.error, nullptr);
        EXPECT_STREQ(c.parse_tree, expected[i]);

        // move
        auto m = std::move(result);
        EXPECT_EQ(m.error, nullptr);
        EXPECT_STREQ(m.parse_tree, expected[i]);

        // move doesn't remove original struct
        EXPECT_EQ(m.error, result.error);
        EXPECT_EQ(m.parse_tree, result.parse_tree);

        pg_query_free_parse_result(result);
    }
}

TEST(QueryTest, ParseDeparse)
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

TEST(QueryTest, ParseUnpack)
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

        PgQueryProtobuf       pbuf         = result.parse_tree;
        PgQuery__ParseResult* parse_result = pg_query__parse_result__unpack(NULL, pbuf.len, (const uint8_t*)pbuf.data);

        pg_query__parse_result__free_unpacked(parse_result, NULL);
        pg_query_free_protobuf_parse_result(result);
    }
}

TEST(QueryTest, ParseModifyDeparse)
{
    // clang-format off
    const char* test_cases[] = {
        // 00: no change
        "SELECT 1",
        // 01: simple example
        "SELECT * FROM x WHERE z = 2",
        // 02: another simple example
        "SELECT * FROM xavier x WHERE z = 2",
        // 03: another simple example
        "SELECT * FROM xavier x WHERE y = 'hello' AND z = 2",
        // 04: self-join example
        "SELECT e.name employee, m.name manager FROM employee e INNER JOIN employee m ON m.employee_id = e.manager_id ORDER BY manager;",
        // 05:
        "SELECT DISTINCT ON (location) location, time, report FROM weather_reports ORDER BY location, time DESC;",
        // 06:
        "SELECT * FROM (SELECT * FROM mytable FOR UPDATE) ss WHERE col1 = 5;",
        // 07:
        "SELECT f.title, f.did, d.name, f.date_prod, f.kind FROM distributors d, films f WHERE f.did = d.did",
        // 08:
        "SELECT kind, sum(len) AS total FROM films GROUP BY kind;",
        // 09:
        "SELECT kind, sum(len) AS total FROM films GROUP BY kind HAVING sum(len) < interval '5 hours';",
        // 10:
        "SELECT distributors.name FROM distributors WHERE distributors.name LIKE 'W%' UNION SELECT actors.name FROM actors WHERE actors.name LIKE 'W%';",
        // 11: try tricking by WITH reusing an existing table name as an alias
        "WITH t AS (SELECT random() as x FROM generate_series(1, 3)) SELECT * FROM t UNION ALL SELECT * FROM t",
        // 12: resursive example
        "WITH RECURSIVE employee_recursive(distance, employee_name, manager_name) AS (SELECT 1, employee_name, manager_name FROM employee WHERE manager_name = 'Mary' UNION ALL SELECT er.distance + 1, e.employee_name, e.manager_name FROM employee_recursive er, employee e WHERE er.employee_name = e.manager_name) SELECT distance, employee_name FROM employee_recursive;",
        // 13: lateral example
        "SELECT m.name AS mname, pname FROM manufacturers m, LATERAL get_product_names(m.id) pname;",
        // 14: another lateral example
        "SELECT m.name AS mname, pname FROM manufacturers m LEFT JOIN LATERAL get_product_names(m.id) pname ON true;",
        // 15: select into a new table
        "select * into xxx_copy from xxx;",
        // 16: create a table
        "CREATE TABLE xxx (ID BIGSERIAL NOT NULL UNIQUE, name TEXT NOT NULL, tags TEXT[] NOT NULL, PRIMARY KEY (id, name));",
        // 17: create a table (if not exists)
        "CREATE TABLE IF NOT EXISTS xxx (id BIGSERIAL PRIMARY KEY, name TEXT NOT NULL, tags TEXT[] NOT NULL);",
        // 18: create a view
        "CREATE VIEW zzz AS SELECT * FROM xxx x JOIN yyy y ON x.signature = y.signature",
        // 19: create or replace a view
        "CREATE OR REPLACE VIEW zzz AS SELECT * FROM xxx x JOIN yyy y ON x.signature = y.signature",
        // 20: drop a view
        "DROP VIEW zzz",
        // 21: drop a view (if exists)
        "DROP VIEW IF EXISTS zzz",
        // 22: drop a table
        "DROP TABLE xxx, yyy, zzz CASCADE",
        // 23: drop a table (if exists)
        "DROP TABLE IF EXISTS xxx",
        // 24: create table as ... (1)
        "CREATE TABLE films_recent AS SELECT * FROM films WHERE date_prod >= '2002-01-01';",
        // 25: create table as ... (2)
        "CREATE TABLE films2 AS TABLE films;",
        // 26: prepare + create temp table
        "PREPARE recentfilms(date) AS SELECT * FROM films WHERE date_prod > $1; CREATE TEMP TABLE films_recent ON COMMIT DROP AS EXECUTE recentfilms('2002-01-01');",
        // 27: current_user #1
        "insert into rental (name) values (current_user);",
        // 28: current_user #2
        "select * from rental where name = current_user;",
        // 29: token-provided principal list
        "select * from rental where name in ('tom', 'g3', 'g2', 'g1');",
        // 30: create or replace trigger
        "CREATE OR REPLACE FUNCTION increment(i integer) RETURNS integer AS $$ BEGIN RETURN i + 1; END; $$ LANGUAGE plpgsql;",
        // 31: set role
        "SET ROLE 'jim@amzn'",
        // 32: acled insert pt.1 (default role binding to the record creator as the owner)
        "with ref as ("
        "   with rec as ("
        "       insert into rental (name) values (current_user) returning id"
        "       )"
        "   insert into __access_binding_refs__ (_origin, _origin_id)"
        "       select 'rental', id from rec returning *"
        "   )"
        "insert into rental__access_bindings__ (_role, _principal, _id, _ref)"
        "   select 'rental_owner', current_user, _origin_id, _id from ref"

        // TODO (M1): SELECT * FROM (INSERT INTO ... RETURNING ...)
    };
    // clang-format on

    auto n = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < n; i++)
    {
        printf("testcase: %02d\n", i);

        // prepare mock schema tracker
        std::shared_ptr<SchemaTracker> st = std::make_shared<SchemaTracker>(nullptr);
        for (auto& rn : {"x", "xavier", "employee", "weather_reports", "mytable", "distributors", "films", "actors",
                         "manufacturers", "xxx", "xxx_copy", "yyy"})
        {
            st->AddRelName(rn);
        }

        // parse query
        auto start  = std::chrono::steady_clock::now();
        auto [q, e] = Query::Parse(test_cases[i]);
        ASSERT_FALSE(e);

        auto end = std::chrono::steady_clock::now();
        auto d   = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << " - parse (ms): " << d.count() << std::endl;

        // json
        start  = std::chrono::steady_clock::now();
        auto j = q.Json();

        end = std::chrono::steady_clock::now();
        d   = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << " - json (ms):  " << d.count() << std::endl;

        // write to file?
        auto write_to_file = true;
        if (write_to_file)
        {
            // output file
            char f[100] = {0};
            sprintf(f, "/workspace/postgresql-proxy/tests/out/parsed_%02d.json", i);
            std::ofstream o(f);
            printf(" output: %s\n", f);

            // pretty printing
            o << j.dump(4) << std::endl;
            o.close();
        }

        // acl query
        QueryAcler acler(st);

        start = std::chrono::steady_clock::now();
        acler.Acl(q);

        end = std::chrono::steady_clock::now();
        d   = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << " - modify (ms):  " << d.count() << std::endl;

        start          = std::chrono::steady_clock::now();
        auto new_query = q.ToString();

        end = std::chrono::steady_clock::now();
        d   = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << " - to_string (ms):  " << d.count() << std::endl;

        // dump json?
        bool                    dump      = false;
        std::unordered_set<int> dump_list = {15, 16, 17, 18, 19, 20, 21, 22, 23};
        if (dump && dump_list.contains(i))
        {
            std::cout << j.dump(4) << std::endl;
        }

        // experiment?
        bool                    experiment      = false;
        std::unordered_set<int> experiment_list = {17};
        if (experiment && experiment_list.contains(i))
        {
            std::cout << "********************" << std::endl;

            auto n = JsonUtil::FindNode(j, "CreateStmt");
            if (n == nullptr)
            {
                std::cout << "Not Found" << std::endl;
                return;
            }

            auto table_name = (*n)["relation"]["relname"].get<std::string>();
            std::cout << table_name << std::endl;

            auto if_not_exists = (*n).contains("if_not_exists");
            if_not_exists      = if_not_exists ? (*n)["if_not_exists"].get<bool>() : false;
            std::cout << if_not_exists << std::endl;

            std::cout << "********************" << std::endl;
            return;
        }

        // raw query string
        printf(" before: %s\n", test_cases[i]);

        // acled query string
        printf(" after:  %s\n", new_query);
        free(new_query);
        printf("-----------------------------------------------------\n");
    }
}

// clang-format off
/*
TEST(QueryTest, TranslateSpecial)
{
    // clang-format off
    std::vector<std::pair<std::string, std::string>> positive_cases = {
        // 00: creating permissions
        {
            "CREATE ACCESS PERMISSION x_view ON x FOR SELECT",
            "INSERT INTO __access_permissions__ (id, name, t_name, op, col_names) VALUES (1, 'x_view', 'x', 'SELECT', ARRAY['*'])"
        },
        // 01:
        {
            "CREATE ACCESS PERMISSION x_view ON x FOR SELECT name, email, order_history",
            "INSERT INTO __access_permissions__ (id, name, t_name, op, col_names) VALUES (1, 'x_view', 'x', 'SELECT', ARRAY['name', 'email', 'order_history'])"
        },
        // 02:
        {
            "CREATE ACCESS PERMISSION x_edit ON x FOR UPDATE",
            "INSERT INTO __access_permissions__ (id, name, t_name, op, col_names) VALUES (2, 'x_edit', 'x', 'UPDATE', ARRAY['*'])"
        },
        // 03:
        {
            "CREATE ACCESS PERMISSION x_extend ON x FOR UPDATE due_date",
            "INSERT INTO __access_permissions__ (id, name, t_name, op, col_names) VALUES (3, 'x_extend', 'x', 'UPDATE', ARRAY['due_date'])"
        },
        // 04:
        {
            "CREATE ACCESS PERMISSION x_rename ON x FOR UPDATE first_name, middle_name, last_name",
            "INSERT INTO __access_permissions__ (id, name, t_name, op, col_names) VALUES (4, 'x_rename', 'x', 'UPDATE', ARRAY['first_name', 'middle_name', 'last_name'])"
        },
        // 05:
        {
            "CREATE ACCESS PERMISSION x_delete ON x FOR DELETE",
            "INSERT INTO __access_permissions__ (id, name, t_name, op) VALUES (5, 'x_delete', 'x', 'DELETE')"
        },
        // 06:
        {
            "CREATE ACCESS PERMISSION x_insert ON x FOR INSERT",
            "INSERT INTO __access_permissions__ (id, name, t_name, op) VALUES (6, 'x_insert', 'x', 'INSERT')"
        },
        // 07:
        {
            "CREATE ACCESS PERMISSION x_all ON x FOR ALL",
            "INSERT INTO __access_permissions__ (id, name, t_name, op) VALUES (7, 'x_all', 'x', 'ALL')"
        },
        // 08: creating roles
        {
            "CREATE ACCESS ROLE staff WITH x_view, x_extend",
            "INSERT INTO __access_roles__ (id, name, perms) VALUES (11, 'staff', ARRAY['x_view', 'x_extend'])"
        },
        // 09:
        {
            "CREATE ACCESS ROLE admin WITH x_view, x_edit, x_insert, x_delete",
            "INSERT INTO __access_roles__ (id, name, perms) VALUES (12, 'admin', ARRAY['x_view', 'x_edit', 'x_insert', 'x_delete'])"
        },
        // 10:
        {
            "CREATE ACCESS ROLE admin WITH x_all",
            "INSERT INTO __access_roles__ (id, name, perms) VALUES (13, 'admin', ARRAY['x_all']')"
        },
        // 11: creating bindings
        {
            "CREATE ACCESS BINDING ROLE staff ON directory WHERE id = 6457 WITH g:rnd",
            "INSERT INTO __access_bindings__ (id, role, t_name, cond, prin) VALUES (345, 'staff', 'directory', 'id = 6457', 'g:rnd')"
        },
        // 12:
        {
            "CREATE ACCESS BINDING ROLE staff ON directory where id = 4567 WITH u:kkwon AS temp_access_kkwon", "INSERT INTO __access_bindings__ (id, role, t_name, cond, prin, alias) VALUES (346, 'staff', 'directory', 'id = 4567', 'u:kkwon', 'temp_access_kkwon')"
        },
        // 13: creating inheritances
        {
            "CREATE ACCESS INHERITANCE FROM directory d TO file f ON d.id = f.dir_id",
            "INSERT INTO __access_inheritances__ (id, src, dst, cond) VALUES (21, 'directory d', 'file f', 'd.id = f.dir_id')"
        },
        // 14:
        {
            "CREATE ACCESS INHERITANCE FROM employee m TO employee e ON m.id = e.manager_id",
            "INSERT INTO __access_inheritances__ (id, src, dst, cond) VALUES (22, 'employee m', 'employee e', 'm.id = e.manager_id')"
        },
        // TODO: create a binding with expiration date
        {
            "",
            ""
        },
        // TODO: create a binding with expiration by duration
        {
            "",
            ""
        },

        // 17: listing permissions
        {
            "LIST ACCESS PERMISSION",
            "SELECT * FROM __access_permissions__"
        },
        // 18:
        {
            "LIST ACCESS PERMISSION ON x",
            "SELECT * FROM __access_permissions__ WHERE t_name = 'x'"
        },
        // 19: listing roles
        {
            "LIST ACCESS ROLE",
            "SELECT * FROM __access_roles__"
        },
        // 20:
        {
            "LIST ACCESS ROLE WITH x_view",
            "SELECT * FROM __access_roles__ WHERE perms @> ARRAY['x_view']"
        },
        // 21:
        {
            "LIST ACCESS ROLE WITH x_all",
            "SELECT * from __access_roles__ WHERE perms @> ARRAY['x_all']"
        },
        // 22:
        {
            "LIST ACCESS ROLE WITH ALL(x_view, x_edit)",
            "SELECT * FROM __access_roles__ WHERE perms @> ARRAY['x_view', 'x_edit']"
        },
        // 23:
        {
            "LIST ACCESS ROLE WITH ANY(x_view, x_edit)",
            "SELECT * FROM __access_roles__ WHERE perms && ARRAY['x_view', 'x_edit'] "
        },

        // TODO: list all roles with any permissions on table x
        {
            "LIST ACCESS ROLE WITH ANY(LIST ACCESS PERMISSION ON x)",   // TBD
            "SELECT * from __access_roles__ WHERE perms && ARRAY(SELECT name FROM __access__permissions__ WHERE t_name = 'x')"
        },
        // TODO: list all roles with any write permissions on table x
        {
            "TBD",  // TBD
            "SELECT * from __access_roles__ WHERE perms && ARRAY(SELECT name FROM __access__permissions__ WHERE t_name = 'x' AND op && ARRAY['ALL', 'INSERT', 'DELETE', 'UPDATE'])"
        },
        //
        {
            "",
            ""
        }

        // TODO: get/set policy
        //      - set is combinatino of CREATE commands
        //      - get could be more convenient
        // TODO: test permissions

        // TODO: more admin-oriented features:
        //      - show all users who has perm p to this resource
        //      - show all users who has any perm to this resource
        //      - show all groups who has perm p1, p2 to this resource
        //      - show all roles with any access to this resource
        //      - show all roles with any access to this table
        //      - show all perms this user has on this resource
        //      - make this access expire in 24 hours
        //      - spoof this user
        //      - approve access upon request (+ set expiration)
        //      - revoke access?
        //      - view acl (instead of table-only acl)?
        //      - function acl?
        //      - listen acl?
        //      - change policy acl? (maybe think in terms of roles)
        //        - who can create permissions?     how do you limit scope?     A: only the db admin and scope is limited the db (or unlimited)
        //        - who can create roles?           how do you limit scope?     A: __access_roles____acl table should have acl data with predefined permissions
        //        - who can create inheritances?    how do you limit scope?     A:
        //        - who can create bindings?        how do you limit scope?     A:

        // references (Organization Role Administrator role):
        //   - iam.roles.create
        //   - iam.roles.delete
        //   - iam.roles.undelete
        //   - iam.roles.get
        //   - iam.roles.list
        //   - iam.roles.update
        //   - resourcemanager.projects.get
        //   - resourcemanager.projects.getIamPolicy
        //   - resourcemanager.projects.list
        //   - resourcemanager.organizations.get
        //   - resourcemanager.organizations.getIamPolicy

        // will probably also need:
        //   - setIamPolicy

        // need predefined permissions
        //   - access_binding_read (equivalent to getIamPolicy)
        //   - access_binding_write (equivalent to setIamPolicy)
        //   - __access_role_all__
        //   - __access_role_insert__

        // need predefined roles
        //   - __access_role_viewer__
        //   - __access_role_editor__
        //   - __access_role_owner/admin__
        //   ......
        //   - __access_binding_viewer__ ()
        //   - __access_binding_editor__ ()


        // (infra) need the following acl tables
        //   -
    };
    // clang-format on

    int i = 0;
    for (auto const& [input, expected] : positive_cases)
    {
        printf("testcase: %02d\n", i++);
        printf(" original:  %s\n", input.c_str());

        // parse as regular query
        EXPECT_THROW(Query _(input.c_str(), nullptr), ParseException);

        // scan as reqular query
        auto result = pg_query_scan(input.c_str());
        EXPECT_EQ(result.error, nullptr);

        auto pbuf        = result.pbuf;
        auto scan_result = pg_query__scan_result__unpack(NULL, pbuf.len, (const uint8_t*)pbuf.data);

        for (int j = 0; j < scan_result->n_tokens; j++)
        {
            auto scan_token   = scan_result->tokens[j];
            auto token_kind   = protobuf_c_enum_descriptor_get_value(&pg_query__token__descriptor,
                                                                     scan_token->token);
            auto keyword_kind = protobuf_c_enum_descriptor_get_value(
                &pg_query__keyword_kind__descriptor, scan_token->keyword_kind);

            auto s = scan_token->start, e = scan_token->end;
            char t[e - s + 3] = {0};
            sprintf(t, "\"%.*s\"", e - s, &(input[s]));

            char tk[13 + strlen(token_kind->name)]   = "token_kind=";
            char kk[13 + strlen(keyword_kind->name)] = "keyword_kind=";

            printf("\t%15s = [%5d,%5d,   %-25s %-35s]\n", t, s, e,
                   strcat(strcat(tk, token_kind->name), ","), strcat(kk, keyword_kind->name));
        }

        pg_query__scan_result__free_unpacked(scan_result, NULL);
        pg_query_free_scan_result(result);

        // TODO: parse as special query

        // token 0 ... n-1
        //  0: token0 = map0[token0]
        //  1: token1 = map1[token1]
        //  2: token2 = map2[token2]

        // maybe no need for any direct validation
        SpecialQuery q(input.c_str());

        // TODO: translate to regular query
        auto translated = q.Translate();

        printf(" translate: %s\n", translated);
        printf("-----------------------------------------------------\n");
    }
}
*/
