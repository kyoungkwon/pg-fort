#include "schema/schema-tracker.h"

const static std::vector<std::string> excluded_ = {
    "__access_bindings__",     "__access_bindings___id_seq",
    "__access_inheritances__", "__access_inheritances___id_seq",
    "__access_permissions__",  "__access_permissions___id_seq",
    "__access_roles__",        "__access_roles___id_seq",
    "gorp_migrations"};

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

    std::unique_lock w_lock(mutex_);
    for (auto row : r)
    {
        relnames_.emplace(row["relname"].c_str());
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
