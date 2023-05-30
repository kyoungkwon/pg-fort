
-- permissions
LIST ACCESS PERMISSION;
LIST ACCESS PERMISSION ON folders;
LIST ACCESS PERMISSION ON documents;

-- roles
LIST ACCESS ROLE;
LIST ACCESS ROLE WITH doc_all;
LIST ACCESS ROLE WITH ALL(folder_view, doc_view);
LIST ACCESS ROLE WITH ANY(doc_edit, doc_create, doc_all);

-- inheritances
LIST ACCESS INHERITANCE;
LIST ACCESS INHERITANCE FROM folders;
LIST ACCESS INHERITANCE TO documents;


-- TODO: formalize the followings:
-- WHO CAN SELECT column c from table t?
-- WHO CAN UPDATE column c from table t?
