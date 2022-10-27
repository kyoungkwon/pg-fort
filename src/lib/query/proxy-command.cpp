#include "query/proxy-command.h"

ProxyCommand::ProxyCommand()
{
}

ProxyCommand::ProxyCommand(const ProxyCommand& c)
    : q_(c.q_)
{
}

ProxyCommand::ProxyCommand(ProxyCommand&& c) noexcept
    : q_(std::move(c.q_))
{
}

ProxyCommand::~ProxyCommand()
{
}

ProxyCommand& ProxyCommand::operator=(const ProxyCommand& other)
{
    q_ = other.q_;
    return *this;
}

ProxyCommand& ProxyCommand::operator=(ProxyCommand&& other)
{
    q_ = std::move(other.q_);
    return *this;
}

ProxyCommand::operator bool() const
{
    return bool(q_);
}

std::pair<ProxyCommand, Error> ProxyCommand::Parse(const char* raw_command)
{
    ProxyCommand c;

    auto [s, err] = RemoveComments(raw_command);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseEnableAccessControl(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseCreateAccessPermission(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseCreateAccessRole(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseCreateAccessInheritance(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseListAccessPermission(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseListAccessRole(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseListAccessInheritance(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseBindAccessRole(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    std::tie(s, err) = ParseUnbindAccessRole(s);
    if (err)
    {
        return {std::move(c), std::move(err)};
    }

    // TODO: add command parsers here

    std::tie(c.q_, err) = Query::Parse(s.c_str());
    return {std::move(c), std::move(err)};
}

char* ProxyCommand::ToString()
{
    return q_.ToString();
}

std::pair<std::string, Error> ProxyCommand::RemoveComments(std::string command)
{
    std::regex re("[\\t\\r\\n]|(--[^\\r\\n]*)|(/\\*[\\w\\W]*?(?=\\*/)\\*/)");
    return {std::regex_replace(command, re, ""), NoError};
}

std::pair<std::string, Error> ProxyCommand::ParseEnableAccessControl(std::string command)
{
    std::regex  re("ENABLE\\s+ACCESS\\s+CONTROL\\s+(\\w+)", std::regex_constants::icase);
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

    return {std::regex_replace(command, re, tpl), NoError};
}

std::pair<std::string, Error> ProxyCommand::ParseCreateAccessPermission(std::string command)
{
    std::regex  re("CREATE\\s+ACCESS\\s+PERMISSION\\s+(\\w+)\\s+ON\\s+(\\w+)\\s+FOR\\s+(\\w+)",
                   std::regex_constants::icase);
    std::string tpl =
        "INSERT INTO __access_permissions__ (name, relation, operation)\n"
        "	VALUES (LOWER('$1'), '$2', UPPER('$3'))";

    return {std::regex_replace(command, re, tpl), NoError};
}

std::pair<std::string, Error> ProxyCommand::ParseCreateAccessRole(std::string command)
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
        translated << "INSERT INTO __access_roles__ (name, permissions) VALUES ('" << r << "', ARRAY[" << p << "]);\n"
                   << "INSERT INTO __access_roles_denorm__ (name, permission)\n"
                   << "	SELECT name, unnest(permissions)\n"
                   << "	FROM __access_roles__\n"
                   << "	WHERE name = '" << r << "'";

        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::ParseCreateAccessInheritance(std::string command)
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
        translated << "INSERT INTO __access_inheritances__ (src, dst, src_query)\n"
                   << "VALUES ('" << f << "', '" << t << "', 'SELECT id FROM " << f;

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

std::pair<std::string, Error> ProxyCommand::ParseListAccessPermission(std::string command)
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
            translated << " WHERE relation = '" << m[2] << "'::REGCLASS";
        }
        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::ParseListAccessRole(std::string command)
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

            translated << " WHERE permissions @> ARRAY[" << p << "]";
        }
        else if (m[6].matched)  // WITH ANY( ... )
        {
            auto p = m[7].str();
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);
            p = std::regex_replace(p, std::regex("(\\w+)"), "'$1'");

            translated << " WHERE permissions && ARRAY[" << p << "]";
        }
        else if (m[2].matched)  // WITH ...
        {
            auto p = m[2].str();
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);

            translated << " WHERE permissions @> ARRAY['" << p << "']";
        }

        command = m.suffix();
    }

    translated << command;
    return {translated.str(), NoError};
}

std::pair<std::string, Error> ProxyCommand::ParseListAccessInheritance(std::string command)
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
                translated << " WHERE src = '" << t << "'::REGCLASS";
            }
            else if (d == "TO")
            {
                translated << " WHERE dst = '" << t << "'::REGCLASS";
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

std::pair<std::string, Error> ProxyCommand::ParseBindAccessRole(std::string command)
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
        "WITH r AS\n"
        "	(INSERT INTO __access_binding_refs__ (origin, origin_id)\n"
        "		VALUES ('$3', ($4))\n"
        "		RETURNING *)\n"
        "INSERT INTO $3__access_bindings__ (role, principal, id, ref)\n"
        "	SELECT '$1', '$2', origin_id, id\n"
        "	FROM r;";

    return {std::regex_replace(command, re, tpl), NoError};
}

std::pair<std::string, Error> ProxyCommand::ParseUnbindAccessRole(std::string command)
{
    // e.g.,
    //  UNBIND ACCESS ROLE doc_viewer
    //       FROM tom@amzn
    //       ON folders (SELECT id FROM folders WHERE name = 'root');
    std::regex re(
        "UNBIND\\s+ACCESS\\s+ROLE\\s+(\\w+)\\s+"
        "FROM\\s+([\\w@]+)\\s+"
        "ON\\s+(\\w+)\\s*\\(([^;]+)\\)",
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
    std::string tpl =
        "DELETE FROM __access_binding_refs__ r\n"
        "	USING $3__access_bindings__ b\n"
        "	WHERE b.id = ($4)\n"
        "	AND b.role = '$1'\n"
        "	AND b.principal = '$2'\n"
        "	AND b.inheritance = 0\n"
        "	AND b.ref = r.id;";

    return {std::regex_replace(command, re, tpl), NoError};
}
