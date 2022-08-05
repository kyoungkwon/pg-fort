#include <pg_query/pg_query.pb-c.h>

void recursion()
{
    PgQuery__ParseResult* parse_result;

    PgQuery__RawStmt** stmts = parse_result->stmts;

    PgQuery__Node* stmt = stmts[0]->stmt;

    PgQuery__Node__NodeCase node_case   = PG_QUERY__NODE__NODE_SELECT_STMT;
    PgQuery__SelectStmt*    select_stmt = stmt->select_stmt;

    size_t          n_from_clause = select_stmt->n_from_clause;
    PgQuery__Node** from_clause   = select_stmt->from_clause;

    PgQuery__Node* where_clause = select_stmt->where_clause;
}

void recursion_from()
{
    PgQuery__Node* from_clause;

    PgQuery__Node__NodeCase node_case = PG_QUERY__NODE__NODE_RANGE_VAR;
    PgQuery__RangeVar*      range_var = from_clause->range_var;

    char* relname = range_var->relname;  // table name

    PgQuery__Alias* alias     = range_var->alias;  // alias is set if not nullptr
    char*           aliasname = alias->aliasname;
}

void recursion_where()
{
    // TODO: not now
    PgQuery__Node* where_clause;

    PgQuery__Node__NodeCase node_case = PG_QUERY__NODE__NODE_A_EXPR;
    PgQuery__AExpr*         a_expr    = where_clause->a_expr;

	
}
