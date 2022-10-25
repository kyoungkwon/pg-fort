
-- create permissions on folders table
CREATE ACCESS PERMISSION folder_view ON folders FOR SELECT;
CREATE ACCESS PERMISSION folder_edit ON folders FOR UPDATE;
CREATE ACCESS PERMISSION folder_create ON folders FOR INSERT;
CREATE ACCESS PERMISSION folder_delete ON folders FOR DELETE;
CREATE ACCESS PERMISSION folder_all ON folders FOR ALL;

-- create permissions on documents table
CREATE ACCESS PERMISSION doc_view ON documents FOR SELECT;
CREATE ACCESS PERMISSION doc_edit ON documents FOR UPDATE;
CREATE ACCESS PERMISSION doc_create ON documents FOR INSERT;
CREATE ACCESS PERMISSION doc_delete ON documents FOR DELETE;
CREATE ACCESS PERMISSION doc_all ON documents FOR ALL;


-- CREATE ACCESS ROLE viewer WITH folder_view, doc_view;
INSERT INTO __access_roles__ (name, permissions) VALUES ('viewer', ARRAY['folder_view', 'doc_view']);
INSERT INTO __access_roles_denorm__ (name, permission)
    SELECT name, unnest(permissions)
    FROM __access_roles__
    WHERE name = 'viewer';

-- CREATE ACCESS ROLE editor WITH folder_view, folder_edit, doc_view, doc_edit;
INSERT INTO __access_roles__ (name, permissions) VALUES ('editor', ARRAY['folder_view', 'folder_edit', 'doc_view', 'doc_edit']);
INSERT INTO __access_roles_denorm__ (name, permission)
    SELECT name, unnest(permissions)
    FROM __access_roles__
    WHERE name = 'editor';

-- CREATE ACCESS ROLE admin WITH folder_all, doc_all;
INSERT INTO __access_roles__ (name, permissions) VALUES ('admin', ARRAY['folder_all', 'doc_all']);
INSERT INTO __access_roles_denorm__ (name, permission)
    SELECT name, unnest(permissions)
    FROM __access_roles__
    WHERE name = 'admin';

-- CREATE ACCESS ROLE doc_viewer WITH doc_view;
INSERT INTO __access_roles__ (name, permissions) VALUES ('doc_viewer', ARRAY['doc_view']);
INSERT INTO __access_roles_denorm__ (name, permission)
    SELECT name, unnest(permissions)
    FROM __access_roles__
    WHERE name = 'doc_viewer';



-- CREATE ACCESS INHERITANCE FROM folders (id) TO folders (parent_id);
INSERT INTO __access_inheritances__ (src, dst, src_query)
    VALUES ('folders', 'folders', 'SELECT id FROM folders WHERE id = $1.parent_id');

-- CREATE ACCESS INHERTINACE FROM folders (id) TO documents (folder_id);
INSERT INTO __access_inheritances__ (src, dst, src_query)
    VALUES ('folders', 'documents', 'SELECT id FROM folders WHERE id = $1.folder_id');
