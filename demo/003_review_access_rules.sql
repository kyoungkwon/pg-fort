
-- permissions
LIST ACCESS PERMISSION;
LIST ACCESS PERMISSION ON folders;
LIST ACCESS PERMISSION ON documents;


-- LIST ACCESS ROLE
SELECT * FROM __access_roles__;

-- LIST ACCESS ROLE WITH doc_all;
SELECT * FROM __access_roles__ WHERE permissions @> ARRAY['doc_all'];

-- LIST ACCESS ROLE WITH ALL(folder_view, doc_view);
SELECT * FROM __access_roles__ WHERE permissions @> ARRAY['folder_view', 'doc_view'];

-- LIST ACCESS ROLE WITH ANY(doc_edit, doc_create, doc_all)
SELECT * FROM __access_roles__ WHERE permissions && ARRAY['doc_edit', 'doc_create', 'doc_all'];



-- LIST INHERITANCE
SELECT * FROM __access_inheritances__;

-- LIST INHERITANCE FROM folders
SELECT * FROM __access_inheritances__ WHERE src = 'folders'::REGCLASS;

-- LIST INHERITANCE TO documents
SELECT * FROM __access_inheritances__ WHERE dst = 'documents'::REGCLASS;



-- TODO: formalize the followings:
-- WHO CAN SELECT column c from table t?
-- WHO CAN UPDATE column c from table t?
