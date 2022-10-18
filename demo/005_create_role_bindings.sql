

-- BIND ACCESS ROLE viewer TO sam@amzn
--      ON folders (SELECT id FROM folders WHERE name = 'folder-a');
WITH r AS
	(INSERT INTO __access_binding_refs__ (origin, origin_id)
		SELECT 'folders', id
		FROM folders
		WHERE name = 'folder-a'
		RETURNING *)
INSERT INTO folders__access_bindings__ (role, principal, id, ref)
	SELECT 'viewer', 'sam@amzn', origin_id, id
	FROM r;


-- BIND ACCESS ROLE editor TO sam@amzn
--      ON folders (SELECT id FROM folders WHERE name = 'folder-d');
WITH r AS
	(INSERT INTO __access_binding_refs__ (origin, origin_id)
		SELECT 'folders', id
		FROM folders
		WHERE name = 'folder-d'
		RETURNING *)
INSERT INTO folders__access_bindings__ (role, principal, id, ref)
	SELECT 'editor', 'sam@amzn', origin_id, id
	FROM r;


-- BIND ACCESS ROLE doc_viewer TO tom@amzn
--      ON folders (SELECT id FROM folders WHERE name = 'root');
WITH r AS
	(INSERT INTO __access_binding_refs__ (origin, origin_id)
		SELECT 'folders', id
		FROM folders
		WHERE name = 'root'
		RETURNING *)
INSERT INTO folders__access_bindings__ (role, principal, id, ref)
	SELECT 'doc_viewer', 'tom@amzn', origin_id, id
	FROM r;

