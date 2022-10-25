
-- admin
SET ROLE myusername;


-- DISABLE ACCESS CONTROL documents
do $$
begin
    EXECUTE format(
        $cmds$
            DROP VIEW IF EXISTS %1$I__acls__;
            DROP TRIGGER IF EXISTS %1$I__upsert__ ON %1$I;
            DROP TABLE IF EXISTS %1$I__access_bindings__;
        $cmds$,
        'documents');
end;
$$;

-- drop documents table
DROP TABLE IF EXISTS documents;


-- DISABLE ACCESS CONTROL folders
do $$
begin
    EXECUTE format(
        $cmds$
            DROP VIEW IF EXISTS %1$I__acls__;
            DROP TRIGGER IF EXISTS %1$I__upsert__ ON %1$I;
            DROP TABLE IF EXISTS %1$I__access_bindings__;
        $cmds$,
        'folders');
end;
$$;

-- drop folders table
DROP TABLE IF EXISTS folders;



-- delete all acl-data
TRUNCATE __access_binding_refs__,
     __access_inheritances__,
     __access_roles__,
     __access_roles_denorm__,
     __access_permissions__
     CASCADE;
