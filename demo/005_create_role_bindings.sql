
-- create user principals
CREATE ROLE "sam@amzn";
CREATE ROLE "tom@amzn";

-- create role bindings
BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');
BIND ACCESS ROLE viewer TO sam@amzn ON folders (SELECT id FROM folders WHERE name = 'folder-a');
BIND ACCESS ROLE editor TO sam@amzn ON folders (SELECT id FROM folders WHERE name = 'folder-d');

-- list role bindings
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'root');
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'folder-a');
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'folder-b');
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'folder-c');
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'folder-d');
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'folder-e');
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'folder-f');
LIST ACCESS ROLE BINDING ON folders (SELECT id FROM folders WHERE name = 'folder-g');
