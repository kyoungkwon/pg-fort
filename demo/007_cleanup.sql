
-- switch to admin
SET ROLE myusername;

-- delete role bindings
do $$
declare
    d RECORD;
    b RECORD;
begin
    FOR d IN
        SELECT * FROM documents
    LOOP
        FOR b IN
            LIST ACCESS ROLE BINDING ON documents (d.id)
        LOOP
            EXECUTE 'UNBIND ACCESS ROLE $1 FROM $2 ON documents ($3)'
                USING b._role, b._principal, b._id;
        END LOOP;
    END LOOP;
end;
$$;

do $$
declare
    f RECORD;
    b RECORD;
begin
    FOR f IN
        SELECT * FROM folders
    LOOP
        FOR b IN
            LIST ACCESS ROLE BINDING ON folders (f.id)
        LOOP
            EXECUTE 'UNBIND ACCESS ROLE $1 FROM $2 ON folders ($3)'
                USING b._role, b._principal, b._id;
        END LOOP;
    END LOOP;
end;
$$;

-- drop roles
DROP ACCESS ROLE viewer;
DROP ACCESS ROLE editor;
DROP ACCESS ROLE admin;
DROP ACCESS ROLE doc_viewer;

-- drop permissions
DROP ACCESS PERMISSION folder_view;
DROP ACCESS PERMISSION folder_edit;
DROP ACCESS PERMISSION folder_create;
DROP ACCESS PERMISSION folder_delete;
DROP ACCESS PERMISSION folder_all;
DROP ACCESS PERMISSION doc_view;
DROP ACCESS PERMISSION doc_edit;
DROP ACCESS PERMISSION doc_create;
DROP ACCESS PERMISSION doc_delete;
DROP ACCESS PERMISSION doc_all;

-- disable access controls
DISABLE ACCESS CONTROL documents;
DISABLE ACCESS CONTROL folders;

-- drop tables
DROP TABLE IF EXISTS documents;
DROP TABLE IF EXISTS folders;


-----------------------------------------------------------

-- switch to admin
SET ROLE myusername;

-- DISABLE ACCESS CONTROL documents
do $$
begin
    EXECUTE format(
        $cmds$
            -- ensures no role is using these permissions, thus no bindings on this table
            -- DELETE FROM __access_permissions__ WHERE relation = %1$L::REGCLASS;
            -- DELETE FROM __access_inheritances__ WHERE src = %1$L::REGCLASS OR dst = %1$L::REGCLASS;
            -- DELETE FROM __access_binding_refs__ WHERE origin = %1$L::REGCLASS;
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
     __access_roles__,
     __access_roles_denorm__,
     __access_permissions__
     CASCADE;

DELETE FROM __access_inheritances__;
