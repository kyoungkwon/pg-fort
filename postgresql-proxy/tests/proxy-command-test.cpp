#include "query/proxy-command.h"

#include <gtest/gtest.h>
#include <pg_query.h>
#include <pg_query/pg_query.pb-c.h>

#include <iostream>
#include <regex>

TEST(ProxyCommandTest, Regex)
{
    std::cout << "========================================" << std::endl;

    std::string input = "ENABLE ACCESS CONTROL folders, documents, symlinks; SELECT * FROM folders;";
    std::regex  re("ENABLE\\s+ACCESS\\s+CONTROL\\s+((\\w+)(,\\s*(\\w+))*)");
    std::string tpl =
        "CREATE TABLE $1__access_bindings__ (\n"
        "	id			BIGINT NOT NULL,\n"
        "	role		TEXT NOT NULL,\n"
        "	principal	TEXT NOT NULL,\n"
        "	ref			BIGINT NOT NULL,\n"
        "	inheritance	BIGINT NOT NULL DEFAULT 0,\n"
        "	ts			TIMESTAMP DEFAULT NOW(),\n"
        "\n"
        "	PRIMARY KEY (id, inheritance, ref),\n"
        "	FOREIGN KEY (id) REFERENCES $1 (id) ON DELETE CASCADE,\n"
        "	FOREIGN KEY (role) REFERENCES __access_roles__ (name) ON DELETE RESTRICT,\n"
        "	FOREIGN KEY (ref) REFERENCES __access_binding_refs__ (id) ON DELETE CASCADE,\n"
        "	FOREIGN KEY (inheritance) REFERENCES __access_inheritances__ (id) ON DELETE CASCADE\n"
        ");\n"
        "\n"
        "CREATE TRIGGER $1__upsert__\n"
        "	AFTER INSERT OR UPDATE ON $1\n"
        "	FOR EACH ROW\n"
        "	EXECUTE FUNCTION __set_access_bindings__();\n"
        "\n"
        "CREATE VIEW $1__acls__ AS\n"
        "	SELECT b.id, b.role, b.principal, r.operation, r.columns\n"
        "	FROM $1__access_bindings__ b, __access_roles_expanded__ r\n"
        "	WHERE r.relation = '$1'::REGCLASS AND b.role = r.name;\n"
        "\n"
        "GRANT ALL PRIVILEGES ON $1, $1__acls__ TO PUBLIC";

    std::cout << std::regex_replace(input, re, tpl) << std::endl;

    std::cout << "========================================" << std::endl;

    std::smatch m;
    if (std::regex_search(input, m, re))
    {
        std::cout << "(" << m.size() << ") " << m[0] << std::endl;
        for (auto i = 1; i < m.size() + 5; i++)
        {
            std::cout << i << ": " << m[i] << " (" << m[i].matched << ")" << std::endl;
        }
    }
    else
    {
        std::cout << "search failed" << std::endl;
    }
}

TEST(ProxyCommandTest, Regex2)
{
    std::cout << "========================================" << std::endl;

    std::regex re(
        "LIST\\s+ACCESS\\s+ROLE"
        "(\\s+WITH\\s+"
        "("
        "(ALL)\\s*\\((\\w+(,\\s*\\w+)*)\\)"
        "|(ANY)\\s*\\((\\w+(,\\s*\\w+)*)\\)"
        "|(\\w+)"
        ")"
        ")?",
        std::regex_constants::icase);

    std::string input =
        "-- nasty combinations...\n"
        "LIST ACCESS ROLE;\n"
        "LIST ACCESS ROLE WITH doc_all;\n"
        "LIST ACCESS ROLE WITH ALL(folder_view, doc_view, xxx);\n"
        "LIST ACCESS ROLE WITH ALL (folder_view, doc_view, yyy);\n"
        "LIST ACCESS ROLE WITH ANY(doc_edit, doc_create, doc_all);\n"
        "LIST ACCESS ROLE WITH ANY  (doc_edit, doc_create, doc_all);\n"
        "LIST ACCESS ROLE WITH ALL(folder_view, doc_view);\n"
        "LIST ACCESS ROLE WITH ALL (folder_view, doc_view);\n";

    std::smatch m;
    while (std::regex_search(input, m, re))
    {
        std::cout << "(" << m.size() << ") " << m[0] << std::endl;
        for (auto i = 1; i < m.size() + 2; i++)
        {
            std::cout << i << ": " << m[i] << " (" << m[i].matched << ")" << std::endl;
        }

        std::cout << "------------------------------\n";
        input = m.suffix();
    }
}

TEST(ProxyCommandTest, Regex3)
{
    std::cout << "========================================" << std::endl;

    std::regex re(
        "LIST\\s+ACCESS\\s+INHERITANCE"
        "(\\s+"
        "(FROM|TO)\\s+(\\w+)"
        ")?",
        std::regex_constants::icase);

    std::string input =
        "LIST ACCESS INHERITANCE;\n"
        "LIST ACCESS INHERITANCE FROM folders;\n"
        "LIST ACCESS INHERITANCE TO documents;";

    std::smatch m;
    while (std::regex_search(input, m, re))
    {
        std::cout << "(" << m.size() << ") " << m[0] << std::endl;
        for (auto i = 1; i < m.size() + 2; i++)
        {
            std::cout << i << ": " << m[i] << " (" << m[i].matched << ")" << std::endl;
        }

        std::cout << "------------------------------\n";
        input = m.suffix();
    }
}

TEST(ProxyCommandTest, Regex4)
{
    std::cout << "========================================" << std::endl;

    std::regex re(
        "BIND\\s+ACCESS\\s+ROLE\\s+(\\w+)\\s+"
        "TO\\s+([\\w@]+)\\s+"
        "ON\\s+(\\w+)\\s*\\(([^;]+)\\)",
        std::regex_constants::icase);

    std::string input =
        "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (11);\n"
        "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');\n"
        "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (11, 55);\n"
        "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (INSERT INTO folders (name) VALUES ('src'); SELECT id FROM "
        "folders WHERE name = 'src');\n";

    std::smatch m;
    while (std::regex_search(input, m, re))
    {
        std::cout << "(" << m.size() << ") " << m[0] << std::endl;
        for (auto i = 1; i < m.size() + 2; i++)
        {
            std::cout << i << ": " << m[i] << " (" << m[i].matched << ")" << std::endl;
        }

        std::cout << "------------------------------\n";
        input = m.suffix();
    }
}

TEST(ProxyCommandTest, Regex5)
{
    std::cout << "========================================" << std::endl;

    std::regex re(
        "UNBIND\\s+ACCESS\\s+ROLE\\s+((\\w+)|(\\$\\d+))\\s+"
        "FROM\\s+(([\\w@]+)|(\\$\\d+))\\s+"
        "ON\\s+((\\w+)|(\\$\\d+))\\s*\\(([^;]+)\\)",
        std::regex_constants::icase);

    std::string input =
        "UNBIND ACCESS ROLE doc_viewer FROM tom@amzn ON folders (11);\n"
        "UNBIND ACCESS ROLE doc_viewer FROM tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');\n"
        "EXECUTE 'UNBIND ACCESS ROLE $1 FROM $2 ON $3 ($4)' USING b.role, b.principal, b.origin, b.id;\n";

    std::smatch m;
    while (std::regex_search(input, m, re))
    {
        std::cout << "(" << m.size() << ") " << m[0] << std::endl;
        for (auto i = 1; i < m.size() + 2; i++)
        {
            std::cout << i << ": " << m[i] << " (" << m[i].matched << ")" << std::endl;
        }

        std::cout << "------------------------------\n";
        input = m.suffix();
    }
}

TEST(ProxyCommandTest, EnableAccessControl)
{
    std::cout << "========================================" << std::endl;

    std::string input =
        "-- enable access control comment will break translation\n"
        "ENABLE ACCESS CONTROL documents;\n"
        "ENABLE ACCESS CONTROL folders;\n"
        "enable access control holy_moly;\n"
        "SELECT * FROM folders;";

    std::cout << "input = " << input << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, CreateAccessPermission)
{
    std::cout << "========================================" << std::endl;

    std::string input =
        "-- create new permissions on folder table\n"
        "CREATE ACCESS PERMISSION folder_view ON folders FOR SELECT;\n"
        "CREATE ACCESS PERMISSION folder_edit ON folders FOR UPDATE;\n"
        "CREATE ACCESS PERMISSION folder_create ON folders FOR INSERT;\n"
        "CREATE ACCESS PERMISSION folder_delete ON folders FOR DELETE;\n"
        "create access permission folder_all ON folders FOR all;";

    std::cout << "input = " << input << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input.c_str());
    if (e)
    {
        std::cout << "TRANSLATION COMMAND FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, CreateAccessRole)
{
    std::cout << "========================================" << std::endl;

    std::string input =
        "-- create access roles\n"
        "SELECT * FROM folders;\n"
        "CREATE ACCESS ROLE viewer WITH folder_view, doc_view, pic_view, aud_view;\n"
        "SELECT * FROM documents;\n"
        "CREATE ACCESS ROLE editor WITH folder_edit, doc_edit, pic_edit, aud_edit;\n"
        "SELECT * FROM pictures;\n"
        "CREATE ACCESS ROLE admin WITH folder_all;\n"
        "SELECT * FROM audios";

    std::cout << "input = " << input << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, CreateAccessInheritance)
{
    std::cout << "========================================" << std::endl;

    std::string input =
        "-- create access inheritances\n"
        "SELECT * FROM folders;\n"
        "CREATE ACCESS INHERITANCE FROM folders (id) TO folders (parent_id);\n"
        "SELECT * FROM documents;\n"
        "create access inheritance from folders(id, name) to documents(folder_id, folder_name);\n"
        "SELECT * FROM pictures;\n";

    std::cout << "input = " << input << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, ListAccessPermission)
{
    std::cout << "========================================" << std::endl;

    std::string input =
        "-- list access permissions\n"
        "table folders;\n"
        "LIST ACCESS PERMISSION;\n"
        "table documents;\n"
        "LIST ACCESS PERMISSION ON folders;\n"
        "table pictures;\n";

    std::cout << "input = " << input << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, ListAccessRole)
{
    std::cout << "========================================" << std::endl;

    std::string input =
        "-- nasty combinations...\n"
        "LIST ACCESS ROLE;\n"
        "LIST ACCESS ROLE WITH doc_all;\n"
        "LIST ACCESS ROLE WITH ALL(folder_view, doc_view, xxx);\n"
        "LIST ACCESS ROLE WITH ALL (folder_view, doc_view, yyy);\n"
        "LIST ACCESS ROLE WITH ANY(doc_edit, doc_create, doc_all);\n"
        "LIST ACCESS ROLE WITH ANY  (doc_edit, doc_create, doc_all);\n"
        "LIST ACCESS ROLE WITH ALL(folder_view, doc_view);\n"
        "LIST ACCESS ROLE WITH ALL (folder_view, doc_view);\n";

    std::cout << "input = " << input << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, ListAccessInheritance)
{
    std::cout << "========================================" << std::endl;

    std::string input =
        "LIST ACCESS INHERITANCE;\n"
        "LIST ACCESS INHERITANCE FROM folders;\n"
        "LIST ACCESS INHERITANCE TO documents;";

    std::cout << "input = " << input << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, BindAccessRole)
{
    std::cout << "========================================" << std::endl;

    std::string input_pass =
        "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (11);\n"
        "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');\n";

    std::cout << "input_pass = " << input_pass << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input_pass.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;

    std::string input_fail = "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (11, 55);\n";

    std::cout << "input_fail = " << input_fail << "\n" << std::endl;

    std::tie(s, e) = ProxyCommand::Translate(input_fail.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;

    input_fail =
        "BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders"
        " (INSERT INTO folders (name) VALUES ('src'); SELECT id FROM folders WHERE name = 'src');\n";

    std::cout << "input_fail = " << input_fail << "\n" << std::endl;

    std::tie(s, e) = ProxyCommand::Translate(input_fail.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, UnbindAccessRole)
{
    std::cout << "========================================" << std::endl;

    std::string input_pass =
        "UNBIND ACCESS ROLE doc_viewer FROM tom@amzn ON folders (11);\n"
        "UNBIND ACCESS ROLE doc_viewer FROM tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');\n"
        "EXECUTE 'UNBIND ACCESS ROLE $1 FROM $2 ON $3 ($4)' USING b.role, b.principal, b.origin, b.id;\n";

    std::cout << "input_pass = " << input_pass << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input_pass.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;

    std::string input_fail = "UNBIND ACCESS ROLE doc_viewer FROM tom@amzn ON folders (11, 55);\n";

    std::cout << "input_fail = " << input_fail << "\n" << std::endl;

    std::tie(s, e) = ProxyCommand::Translate(input_fail.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;

    input_fail =
        "UNBIND ACCESS ROLE doc_viewer FROM tom@amzn ON folders"
        " (INSERT INTO folders (name) VALUES ('src'); SELECT id FROM folders WHERE name = 'src');\n";

    std::cout << "input_fail = " << input_fail << "\n" << std::endl;

    std::tie(s, e) = ProxyCommand::Translate(input_fail.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

TEST(ProxyCommandTest, ListAccessRoleBinding)
{
    std::cout << "========================================" << std::endl;

    std::string input_pass = "LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'root');";

    std::cout << "input_pass = " << input_pass << "\n" << std::endl;

    auto [s, e] = ProxyCommand::Translate(input_pass.c_str());
    if (e)
    {
        std::cout << "TRANSLATION FAILED" << std::endl;
    }
    else
    {
        std::cout << s << std::endl;
    }

    std::cout << "========================================" << std::endl;
}

// clang-format off
/*
TEST(QueryTest, TranslateSpecial)
{
    // clang-format off
    std::vector<std::pair<std::string, std::string>> positive_cases = {
        // 00: creating permissions
        {
            "ENABLE ACCESS CONTROL x; CREATE ACCESS PERMISSION x_view ON x FOR SELECT",
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

        // scan as reqular query
        auto result = pg_query_scan(input.c_str());
        EXPECT_EQ(result.error, nullptr);

        auto pbuf        = result.pbuf;
        auto scan_result = pg_query__scan_result__unpack(NULL, pbuf.len, (const uint8_t*)pbuf.data);

        for (int j = 0; j < scan_result->n_tokens; j++)
        {
            auto scan_token = scan_result->tokens[j];
            auto token_kind = protobuf_c_enum_descriptor_get_value(&pg_query__token__descriptor, scan_token->token);
            auto keyword_kind =
                protobuf_c_enum_descriptor_get_value(&pg_query__keyword_kind__descriptor, scan_token->keyword_kind);

            auto s = scan_token->start, e = scan_token->end;
            char t[e - s + 3] = {0};
            sprintf(t, "\"%.*s\"", e - s, &(input[s]));

            char tk[13 + strlen(token_kind->name)]   = "token_kind=";
            char kk[13 + strlen(keyword_kind->name)] = "keyword_kind=";

            printf("\t%15s = [%5d,%5d,   %-25s %-35s]\n", t, s, e, strcat(strcat(tk, token_kind->name), ","),
                   strcat(kk, keyword_kind->name));
        }

        pg_query__scan_result__free_unpacked(scan_result, NULL);
        pg_query_free_scan_result(result);
    }
}
*/
