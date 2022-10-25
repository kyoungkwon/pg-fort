#include "schema/schema-tracker.h"

// clang-format off
const static std::unordered_set<std::string> excluded_ = {
    "__access_binding_refs__",
    "__access_inheritances__",
    "__access_permissions__",
    "__access_roles__",
    "__access_roles_denorm__",
    "__access_roles_expanded__",
    "gorp_migrations"
};
// clang-format on

SchemaTracker::SchemaTracker(std::shared_ptr<PqxxConnPool> pcp)
    : pcp_(pcp)
{
    Refresh();
}

SchemaTracker::~SchemaTracker()
{
}

void SchemaTracker::Refresh()
{
    if (pcp_ == nullptr)
    {
        return;
    }

    auto       c = pcp_->Acquire();
    pqxx::work w(*c);

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
        " FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = "
        "c.relnamespace"
        " WHERE c.relkind = ANY(ARRAY['r','p','v','m','f'])"
        " AND pg_catalog.pg_table_is_visible(c.oid)"
        " AND n.nspname <> 'pg_catalog'"
        " AND n.nspname <> 'information_schema'"
        " AND n.nspname !~ '^pg_toast'");

    w.commit();

    std::unique_lock w_lock(mutex_);
    for (auto row : r)
    {
        auto relname = std::string(row["relname"].c_str());
        if (!relname.ends_with("__acl__") && !relname.ends_with("__access_bindings__"))
        {
            relnames_.emplace(relname);
        }
    }
}

void SchemaTracker::AddRelName(std::string relname)
{
    std::unique_lock w_lock(mutex_);
    relnames_.emplace(relname);
}

bool SchemaTracker::Exist(std::string relname)
{
    std::shared_lock r_lock(mutex_);
    return !excluded_.contains(relname) && relnames_.contains(relname);
}
