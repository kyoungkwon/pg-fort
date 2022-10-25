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
    Error        err;

    auto s = RemoveComments(raw_command);
    s      = ParseEnableAccessControl(s);
    s      = ParseCreateAccessPermission(s);
    s      = ParseCreateAccessRole(s);

    // TODO

    std::tie(c.q_, err) = Query::Parse(s.c_str());
    return {std::move(c), std::move(err)};
}

char* ProxyCommand::ToString()
{
    return q_.ToString();
}

std::string ProxyCommand::RemoveComments(std::string command)
{
    std::regex re("[\\t\\r\\n]|(--[^\\r\\n]*)|(/\\*[\\w\\W]*?(?=\\*/)\\*/)");
    return std::regex_replace(command, re, "");
}

std::string ProxyCommand::ParseEnableAccessControl(std::string command)
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

    return std::regex_replace(command, re, tpl);
}

std::string ProxyCommand::ParseCreateAccessPermission(std::string command)
{
    std::regex  re("CREATE\\s+ACCESS\\s+PERMISSION\\s+(\\w+)\\s+ON\\s+(\\w+)\\s+FOR\\s+(\\w+)",
                   std::regex_constants::icase);
    std::string tpl =
        "INSERT INTO __access_permissions__ (name, relation, operation)\n"
        "	VALUES ('$1', '$2', UPPER('$3'))";

    return std::regex_replace(command, re, tpl);
}

std::string ProxyCommand::ParseCreateAccessRole(std::string command)
{
    std::regex  re("CREATE\\s+ACCESS\\s+ROLE\\s+(\\w+)\\s+WITH\\s+(\\w+(,\\s*\\w+)*)", std::regex_constants::icase);
    std::smatch m;

    std::ostringstream translated;
    while (std::regex_search(command, m, re))
    {
        auto r = m[1].str();
        auto p = std::regex_replace(m[2].str(), std::regex("(\\w+)"), "'$1'");

        translated << m.prefix();
        translated << "INSERT INTO __access_roles__ (name, permissions) VALUES ('" << r << "', ARRAY[" << p << "]);\n";
        translated << "INSERT INTO __access_roles_denorm__ (name, permission)\n"
                   << "	SELECT name, unnest(permissions)\n"
                   << "	FROM __access_roles__\n"
                   << "	WHERE name = '" << r << "'";

        command = m.suffix();
    }
    translated << command;
    return translated.str();
}
