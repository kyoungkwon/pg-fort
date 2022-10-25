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

    // TODO

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
        "	VALUES ('$1', '$2', UPPER('$3'))";

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
        auto p = std::regex_replace(m[2].str(), std::regex("(\\w+)"), "'$1'");

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
