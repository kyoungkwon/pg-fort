
SELECT session_user, current_user;

-- switch user
SET ROLE "sam@amzn";
SET ROLE "tom@amzn";

-- change role bindings for tom@amzn
SET ROLE myusername;
UNBIND ACCESS ROLE doc_viewer FROM tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');

SET ROLE "tom@amzn";
SELECT * FROM folders;
SELECT * FROM documents;

SET ROLE myusername;
BIND ACCESS ROLE viewer TO tom@amzn ON folders (SELECT id FROM folders WHERE name = 'folder-b');

SET ROLE "tom@amzn";
SELECT * FROM folders;
SELECT * FROM documents;
