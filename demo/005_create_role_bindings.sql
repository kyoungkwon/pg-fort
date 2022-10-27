
-- create user principals
CREATE ROLE "sam@amzn";
CREATE ROLE "tom@amzn";

-- create role bindings
BIND ACCESS ROLE viewer TO sam@amzn ON folders (SELECT id FROM folders WHERE name = 'folder-a');
BIND ACCESS ROLE editor TO sam@amzn ON folders (SELECT id FROM folders WHERE name = 'folder-d');
BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');
