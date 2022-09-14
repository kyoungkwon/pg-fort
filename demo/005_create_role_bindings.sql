

-- CREATE ACCESS BINDING FOR sam ON folders WHERE name = 'folder-a' WITH viewer;
INSERT INTO __access_bindings__ (role, relation, condition, principal) VALUES ('viewer', 'folders', "name = 'folder-a'", 'sam');

-- CREATE ACCESS BINDING FOR sam ON folders WHERE name = 'folder-d' WITH editor;
INSERT INTO __access_bindings__ (role, relation, condition, principal) VALUES ('editor', 'folders', "name = 'folder-d", 'sam');

-- CREATE ACCESS BINDING FOR tom ON folders WHERE name = 'root' WITH doc_viewer;
INSERT INTO __access_bindings__ (role, relation, condition, principal) VALUES ('doc_viewer', 'folders', "name = 'root", 'tom');
