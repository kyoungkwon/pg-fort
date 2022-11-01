#include "query/proxy-command.h"

// TODO:
// the current implementation makes many passes over the query string,
// it should be improved to minimize the number of passes
std::pair<std::string, Error> ProxyCommand::Translate(const char* query)
{
    auto [s, err] = RemoveComments(query);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateEnableAccessControl(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateCreateAccessPermission(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateCreateAccessRole(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateCreateAccessInheritance(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateListAccessPermission(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateListAccessRoleBinding(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateListAccessRole(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateListAccessInheritance(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateBindAccessRole(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    std::tie(s, err) = TranslateUnbindAccessRole(s);
    if (err)
    {
        return {query, std::move(err)};
    }

    // TODO: add command parsers here

    return {std::move(s), NoError};
}

std::pair<std::string, Error> ProxyCommand::RemoveComments(std::string command)
{
    std::regex re("(--[^\\r\\n]*)|(/\\*[\\w\\W]*?(?=\\*/)\\*/)");
    return {std::regex_replace(command, re, ""), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateEnableAccessControl(std::string command)
{
    std::regex  re("ENABLE\\s+ACCESS\\s+CONTROL\\s+(\\w+)", std::regex_constants::icase);
    std::string tpl =
        "CREATE TABLE $1__access_bindings__ ("
        "   _id             BIGINT NOT NULL,"
        "   _role           TEXT NOT NULL,"
        "   _principal      TEXT NOT NULL,"
        "   _ref            BIGINT NOT NULL,"
        "   _inheritance    BIGINT NOT NULL DEFAULT 0,"
        "   _ts             TIMESTAMP DEFAULT NOW(),"
        ""
        "   PRIMARY KEY (_id, _inheritance, _ref),"
        "   FOREIGN KEY (_id) REFERENCES $1 (id) ON DELETE CASCADE,"
        "   FOREIGN KEY (_role) REFERENCES __access_roles__ (_name) ON DELETE RESTRICT,"
        "   FOREIGN KEY (_ref) REFERENCES __access_binding_refs__ (_id) ON DELETE CASCADE,"
        "   FOREIGN KEY (_inheritance) REFERENCES __access_inheritances__ (_id) ON DELETE CASCADE"
        ");"
        ""
        "CREATE TRIGGER $1__upsert__"
        "   AFTER INSERT OR UPDATE ON $1"
        "   FOR EACH ROW"
        "   EXECUTE FUNCTION __set_access_bindings__();"
        ""
        "CREATE VIEW $1__acls__ AS"
        "   SELECT b._id, b._role, b._principal, r._operation, r._columns"
        "   FROM $1__access_bindings__ b, __access_roles_expanded__ r"
        "   WHERE r._relation = '$1'::REGCLASS AND b._role = r._name;"
        ""
        "GRANT ALL PRIVILEGES ON $1, $1__acls__ TO PUBLIC";

    return {std::regex_replace(command, re, tpl), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateCreateAccessPermission(std::string command)
{
    std::regex  re("CREATE\\s+ACCESS\\s+PERMISSION\\s+(\\w+)\\s+ON\\s+(\\w+)\\s+FOR\\s+(\\w+)",
                   std::regex_constants::icase);
    std::string tpl =
        "INSERT INTO __access_permissions__ (_name, _relation, _operation)"
        "	VALUES (LOWER('$1'), '$2', UPPER('$3'))";

    return {std::regex_replace(command, re, tpl), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateCreateAccessRole(std::string command)
{
    std::regex  re("CREATE\\s+ACCESS\\s+ROLE\\s+(\\w+)\\s+WITH\\s+(\\w+(,\\s*\\w+)*)", std::regex_constants::icase);
    std::smatch m;

    std::ostringstream translated;
    while (std::regex_search(command, m, re))
    {
        auto r = m[1].str();
        std::transform(r.begin(), r.end(), r.begin(), ::tolower);

        auto p = m[2].str();
        std::transform(p.begin(), p.end(), p.begin(), ::tolower);
        p = std::regex_replace(p, std::regex("(\\w+)"), "'$1'");

        translated << m.prefix();
        translated << "INSERT INTO __access_roles__ (_name, _permissions) VALUES ('" << r << "', ARRAY[" << p << "]);"
                   << "INSERT INTO __access_roles_denorm__ (_name, _permission)"
                   << " SELECT _name, unnest(_permissions)"
                   << " FROM __access_roles__"
                   << " WHERE _name = '" << r << "'";

        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateCreateAccessInheritance(std::string command)
{
    std::regex re(
        "CREATE\\s+ACCESS\\s+INHERITANCE\\s+"
        "FROM\\s+(\\w+)\\s*\\((\\w+(,\\s*\\w+)*)\\)\\s+"
        "TO\\s+(\\w+)\\s*\\((\\w+(,\\s*\\w+)*)\\)",
        std::regex_constants::icase);

    std::smatch        m;
    std::ostringstream translated;
    while (std::regex_search(command, m, re))
    {
        auto f = m[1].str();
        auto t = m[4].str();

        translated << m.prefix();
        translated << "INSERT INTO __access_inheritances__ (_src, _dst, _src_query)"
                   << " VALUES ('" << f << "', '" << t << "', 'SELECT id FROM " << f;

        auto        f_cols = m[2].str();
        auto        t_cols = m[5].str();
        std::smatch f_cols_m;
        std::smatch t_cols_m;

        std::regex w("\\w+");

        auto f_next = std::regex_search(f_cols, f_cols_m, w);
        auto t_next = std::regex_search(t_cols, t_cols_m, w);
        bool first  = true;
        for (; f_next && t_next; f_next = std::regex_search(f_cols, f_cols_m, w),
                                 t_next = std::regex_search(t_cols, t_cols_m, w), first = false)
        {
            translated << (first ? " WHERE " : " AND ") << f_cols_m.str() << " = $1." << t_cols_m.str();
            f_cols = f_cols_m.suffix();
            t_cols = t_cols_m.suffix();
        }
        translated << "')";

        if (f_next != t_next)
        {
            Error err = {
                {"S",                                                     "ERROR"},
                {"M", "dimension mismatch between source and destination columns"},
                {"R",                                                    __func__},
                {"F",                                                    __FILE__},
                {"L",                                    std::to_string(__LINE__)}
            };
            return {std::string(), std::move(err)};
        }

        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateListAccessPermission(std::string command)
{
    std::regex re("LIST\\s+ACCESS\\s+PERMISSION(\\s+ON\\s+(\\w+))?", std::regex_constants::icase);

    std::smatch        m;
    std::ostringstream translated;
    while (std::regex_search(command, m, re))
    {
        translated << m.prefix();
        translated << "SELECT * FROM __access_permissions__";
        if (m[2].matched)
        {
            translated << " WHERE _relation = '" << m[2] << "'::REGCLASS";
        }
        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateListAccessRole(std::string command)
{
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

    std::smatch        m;
    std::ostringstream translated;
    while (std::regex_search(command, m, re))
    {
        translated << m.prefix();
        translated << "SELECT * FROM __access_roles__";

        if (m[3].matched)  // WITH ALL( ... )
        {
            auto p = m[4].str();
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);
            p = std::regex_replace(p, std::regex("(\\w+)"), "'$1'");

            translated << " WHERE _permissions @> ARRAY[" << p << "]";
        }
        else if (m[6].matched)  // WITH ANY( ... )
        {
            auto p = m[7].str();
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);
            p = std::regex_replace(p, std::regex("(\\w+)"), "'$1'");

            translated << " WHERE _permissions && ARRAY[" << p << "]";
        }
        else if (m[2].matched)  // WITH ...
        {
            auto p = m[2].str();
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);

            translated << " WHERE _permissions @> ARRAY['" << p << "']";
        }

        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateListAccessInheritance(std::string command)
{
    std::regex re(
        "LIST\\s+ACCESS\\s+INHERITANCE"
        "(\\s+"
        "(FROM|TO)\\s+(\\w+)"
        ")?",
        std::regex_constants::icase);

    std::smatch        m;
    std::ostringstream translated;
    while (std::regex_search(command, m, re))
    {
        translated << m.prefix();
        translated << "SELECT * FROM __access_inheritances__";

        if (m[2].matched)
        {
            auto d = m[2].str();
            std::transform(d.begin(), d.end(), d.begin(), ::toupper);

            auto t = m[3].str();
            if (d == "FROM")
            {
                translated << " WHERE _src = '" << t << "'::REGCLASS";
            }
            else if (d == "TO")
            {
                translated << " WHERE _dst = '" << t << "'::REGCLASS";
            }
            else
            {
                Error err = {
                    {"S",                                 "ERROR"},
                    {"M", "syntax error at or near \"" + d + "\""},
                    {"R",                                __func__},
                    {"F",                                __FILE__},
                    {"L",                std::to_string(__LINE__)}
                };
                return {std::string(), std::move(err)};
            }
        }

        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateBindAccessRole(std::string command)
{
    // e.g.,
    //  BIND ACCESS ROLE doc_viewer
    //       TO tom@amzn
    //       ON folders (SELECT id FROM folders WHERE name = 'root');
    std::regex re(
        "BIND\\s+ACCESS\\s+ROLE\\s+(\\w+)\\s+"
        "TO\\s+([\\w@]+)\\s+"
        "ON\\s+(\\w+)\\s*\\(([^;]+)\\)",
        std::regex_constants::icase);

    // $1 = doc_viewer
    // $2 = tom@amzn
    // $3 = folders
    // $4 = SELECT id FROM folders WHERE name = 'root'

    // e.g.,
    // WITH r AS
    // 	(INSERT INTO __access_binding_refs__ (origin, origin_id)
    // 		VALUES ('folders', (SELECT folders.id FROM folders WHERE name = 'root'))
    // 		RETURNING *)
    // INSERT INTO folders__access_bindings__ (role, principal, id, ref)
    // 	SELECT 'doc_viewer', 'tom@amzn', origin_id, id
    // 	FROM r;
    std::string tpl =
        "WITH r AS"
        "   (INSERT INTO __access_binding_refs__ (_origin, _origin_id)"
        "       VALUES ('$3', ($4))"
        "       RETURNING *)"
        "INSERT INTO $3__access_bindings__ (_role, _principal, _id, _ref)"
        "   SELECT '$1', '$2', _origin_id, _id"
        "   FROM r";

    return {std::regex_replace(command, re, tpl), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateUnbindAccessRole(std::string command)
{
    // e.g.,
    //  UNBIND ACCESS ROLE doc_viewer
    //       FROM tom@amzn
    //       ON folders (SELECT id FROM folders WHERE name = 'root');
    std::regex re(
        "UNBIND\\s+ACCESS\\s+ROLE\\s+((\\w+)|(\\$\\d+))\\s+"
        "FROM\\s+(([\\w@]+)|(\\$\\d+))\\s+"
        "ON\\s+((\\w+)|(\\$\\d+))\\s*\\(([^;]+)\\)",
        std::regex_constants::icase);

    // $1 = doc_viewer
    // $2 = tom@amzn
    // $3 = folders
    // $4 = SELECT id FROM folders WHERE name = 'root'

    // e.g.,
    // DELETE FROM __access_binding_refs__ r
    //  USING folders__access_bindings__ b
    //  WHERE b.id = (SELECT id FROM folders WHERE name = 'root')
    //  AND b.role = 'doc_viewer'
    //  AND b.principal = 'tom@amzn'
    //  AND b.inheritance = 0
    //  AND b.ref = r.id;
    std::smatch        m;
    std::ostringstream translated;
    while (std::regex_search(command, m, re))
    {
        translated << m.prefix();

        // TODO: fix $3 and $4
        translated << "DELETE FROM __access_binding_refs__ r"
                   << " USING " << m[7].str() << "__access_bindings__ b"
                   << " WHERE b._id = (" << m[10].str() << ")";

        if (m[2].matched)
        {
            translated << " AND b._role = '" << m[2].str() << "'";
        }
        else if (m[3].matched)
        {
            translated << " AND b._role = " << m[3].str();
        }

        if (m[5].matched)
        {
            translated << " AND b._principal = '" << m[5].str() << "'";
        }
        else if (m[6].matched)
        {
            translated << " AND b._principal = " << m[6].str();
        }

        translated << " AND b._inheritance = 0"
                   << " AND b._ref = r._id";

        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::TranslateListAccessRoleBinding(std::string command)
{
    // e.g.,
    // LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'root');
    std::regex re(
        "LIST\\s+ACCESS\\s+ROLE\\s+BINDING\\s+"
        "ON\\s+(\\w+)\\s*\\(([^;]+)\\)",
        std::regex_constants::icase);

    // $1 = folders
    // $2 = SELECT id FROM folders WHERE name = 'root'

    // e.g.,
    // SELECT *
    //  FROM folders__access_bindings__
    //  WHERE id = (SELECT id FROM folders WHERE name = 'root');
    std::string tpl =
        "SELECT *"
        "   FROM $1__access_bindings__"
        "   WHERE _id = ($2)";

    return {std::regex_replace(command, re, tpl), NoError};
}
