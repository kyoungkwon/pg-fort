
-- DISABLE ACCESS CONTROL folders
do $$
begin
    EXECUTE format(
        $cmds$
            DROP VIEW %1$I__acls__;
            DROP TRIGGER %1$I__upsert__ ON %1$I;
            DROP TABLE %1$I__access_bindings__;
        $cmds$,
        'folders');
end;
$$;


-- DISABLE ACCESS CONTROL documents
do $$
begin
    EXECUTE format(
        $cmds$
            DROP VIEW %1$I__acls__;
            DROP TRIGGER %1$I__upsert__ ON %1$I;
            DROP TABLE %1$I__access_bindings__;
        $cmds$,
        'documents');
end;
$$;


-- drop user-data schema
DROP TABLE documents;
DROP TABLE folders;
