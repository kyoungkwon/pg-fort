
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


-- create roles
CREATE ACCESS ROLE viewer WITH folder_view, doc_view;
CREATE ACCESS ROLE editor WITH folder_view, folder_edit, doc_view, doc_edit;
CREATE ACCESS ROLE admin WITH folder_all, doc_all;
CREATE ACCESS ROLE doc_viewer WITH doc_view;


-- create inheritances
CREATE ACCESS INHERITANCE FROM folders (id) TO folders (parent_id);
CREATE ACCESS INHERITANCE FROM folders (id) TO documents (folder_id);
