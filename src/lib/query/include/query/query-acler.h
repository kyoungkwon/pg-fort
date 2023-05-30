#ifndef __PG_FORT_QUERY_ACLER_H__
#define __PG_FORT_QUERY_ACLER_H__

#include "query/query.h"
#include "schema/schema-tracker.h"

class QueryAcler
{
private:
    std::shared_ptr<SchemaTracker> st_;

    void Acl(json& j);
    void AclSelectStmt(json& j);
    void AclSelectStmtFromClause(json& j);

public:
    QueryAcler(std::shared_ptr<SchemaTracker> st);

    void Acl(Query& q);
};

#endif
