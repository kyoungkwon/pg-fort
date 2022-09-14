

-- CREATE ACCESS PERMISSION folder_view ON folders FOR SELECT;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('folder_view', 'folders', 'SELECT');

-- CREATE ACCESS PERMISSION folder_edit ON folders FOR UPDATE;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('folder_edit', 'folders', 'UPDATE');

-- CREATE ACCESS PERMISSION folder_create ON folders FOR INSERT;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('folder_create', 'folders', 'INSERT');

-- CREATE ACCESS PERMISSION folder_delete ON folders FOR DELETE;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('folder_delete', 'folders', 'DELETE');

-- CREATE ACCESS PERMISSION folder_all ON folders FOR ALL;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('folder_all', 'folders', 'ALL');

-- CREATE ACCESS PERMISSION doc_view ON documents FOR SELECT;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('doc_view', 'documents', 'SELECT');

-- CREATE ACCESS PERMISSION doc_edit ON documents FOR UPDATE;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('doc_edit', 'documents', 'UPDATE');

-- CREATE ACCESS PERMISSION doc_create ON documents FOR INSERT;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('doc_create', 'documents', 'INSERT');

-- CREATE ACCESS PERMISSION doc_delete ON documents FOR DELETE;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('doc_delete', 'documents', 'DELETE');

-- CREATE ACCESS PERMISSION doc_all ON documents FOR ALL;
INSERT INTO __access_permissions__ (name, relation, operation) VALUES ('doc_all', 'documents', 'ALL');



-- CREATE ACCESS ROLE viewer WITH folder_view, doc_view;
INSERT INTO __access_roles__ (name, permissions) VALUES ('viewer', ARRAY['folder_view', 'doc_view']);
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('viewer', 'folder_view');
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('viewer', 'doc_view');

-- CREATE ACCESS ROLE editor WITH folder_view, folder_edit, doc_view, doc_edit;
INSERT INTO __access_roles__ (name, permissions) VALUES ('editor', ARRAY['folder_view', 'folder_edit', 'doc_view', 'doc_edit']);
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('editor', 'folder_view');
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('editor', 'folder_edit');
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('editor', 'doc_view');
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('editor', 'doc_edit');

-- CREATE ACCESS ROLE admin WITH folder_all, doc_all;
INSERT INTO __access_roles__ (name, permissions) VALUES ('admin', ARRAY['folder_all', 'doc_all']);
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('admin', 'folder_all');
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('admin', 'doc_all');

-- CREATE ACCESS ROLE doc_viewer WITH doc_view;
INSERT INTO __access_roles__ (name, permissions) VALUES ('doc_viewer', ARRAY['doc_view']);
INSERT INTO __access_roles_denorm__ (name, permission) VALUES ('doc_viewer', 'doc_view');



-- CREATE ACCESS INHERITANCE FROM folders p TO folders c ON p.id = c.parent_id;
INSERT INTO __access_inheritances__ (source, destination, condition) VALUES ('folders p', 'folders c', 'p.id = c.parent_id');

-- CREATE ACCESS INHERTINACE FROM folders f TO documents d ON f.id = d.folder_id;
INSERT INTO __access_inheritances__ (source, destination, condition) VALUES ('folders f', 'documents d', 'f.id = d.folder_id');

