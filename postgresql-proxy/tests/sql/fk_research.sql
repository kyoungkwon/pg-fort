
create table test_a (
	i	int,
	t	text,
	b   bool,

	PRIMARY KEY (i, t, b)
);

create table test_b (
	id	BIGSERIAL,
	x	int,
	y	text,
	z	bool,

	PRIMARY KEY (id),
	FOREIGN KEY (x, y, z) REFERENCES test_a ON DELETE RESTRICT
);

select conname, conrelid, confrelid, conkey, confkey
	from pg_catalog.pg_constraint r
	where r.contype = 'f';


select c.confrelid::regclass::text as src_table,
	   a.attname as src_col
	from pg_constraint c join pg_attribute a 
		on a.attrelid = c.confrelid and a.attnum = ANY(c.confkey)
	where c.conname = 'test_b_x_y_z_fkey';


select c.conrelid::regclass::text as dst_table,
	   a.attname as dst_col
	from pg_constraint c join pg_attribute a 
		on a.attrelid = c.conrelid and a.attnum = ANY(c.conkey)
	where c.conname = 'test_b_x_y_z_fkey';


select c.confrelid::regclass::text as src_table,
	   a1.attname as src_col,
	   c.conrelid::regclass::text as dst_table,
	   a2.attname as dst_col
	from pg_constraint c, pg_attribute a1, pg_attribute a2
	where c.conname = 'test_b_x_y_z_fkey'
		and c.confrelid = a1.attrelid and a1.attnum = ANY(c.confkey)
		and c.conrelid = a2.attrelid and a2.attnum = ANY(c.conkey);


SELECT c.confrelid::regclass::text as src_table,
       unnest(c.confkey) AS src_col,
       c.conrelid::regclass::text as dst_table,
       unnest(c.conkey) AS dst_col
	FROM pg_constraint c
	WHERE conname = 'test_b_x_y_z_fkey'


WITH oids AS (
	SELECT c.confrelid as st,
           unnest(c.confkey) AS sc,
           c.conrelid as dt,
           unnest(c.conkey) AS dc
    	FROM pg_constraint c
    	WHERE conname = 'test_b_x_y_z_fkey')
SELECT st::regclass::text as src_table,
	   a1.attname as src_col,
	   dt::regclass::text as dst_table,
	   a2.attname as dst_col
	FROM oids, pg_attribute a1, pg_attribute a2
	WHERE st = a1.attrelid AND a1.attnum = sc
	  AND dt = a2.attrelid AND a2.attnum = dc;



SELECT c.confrelid::regclass::text as src_table,
       c.confkey AS src_col,
       c.conrelid::regclass::text as dst_table,
       c.conkey AS dst_col
	FROM pg_constraint c
	WHERE conname = 'test_b_x_y_z_fkey'





------------------------------------------------------

/*

+-----------+---------+-----------+---------+
| src_table | src_col | dst_table | dst_col |
|-----------+---------+-----------+---------|
| test_a    | i       | test_b    | x       |
| test_a    | t       | test_b    | y       |
| test_a    | b       | test_b    | z       |
+-----------+---------+-----------+---------+

select test_a.id
	from test_a, test_b
	where test_a.i = test_b.x
	  and test_a.t = test_b.y
	  and test_a.b = test_b.z
	  and test_b.id = NEW.id;





mydatabase> select conname, conrelid, confrelid, conkey, confkey from pg_catalog.pg_constraint r where r.contype = 'f';
+-----------------------------------------+----------+-----------+--------+---------+
| conname                                 | conrelid | confrelid | conkey | confkey |
|-----------------------------------------+----------+-----------+--------+---------|
| __access_roles_denorm___name_fkey       | 16414    | 16404     | [1]    | [2]     |
| __access_roles_denorm___permission_fkey | 16414    | 16393     | [2]    | [2]     |
| __access_bindings___role_fkey           | 16432    | 16404     | [2]    | [2]     |
| folders_parent_id_fkey                  | 33457    | 33457     | [3]    | [1]     |
| documents_folder_id_fkey                | 33473    | 33457     | [4]    | [1]     |
| example__acl___id_fkey                  | 24917    | 24907     | [1]    | [1]     |
| example__acl___permission_fkey          | 24917    | 16393     | [2]    | [2]     |
| aaa_bindings_binding_def_fkey           | 33365    | 33337     | [1]    | [1]     |
| aaa_bindings_id_fkey                    | 33365    | 33357     | [2]    | [1]     |
| bbb_bindings_binding_def_fkey           | 33389    | 33337     | [1]    | [1]     |
| bbb_bindings_id_fkey                    | 33389    | 33381     | [2]    | [1]     |
| xxx_bindings_binding_def_fkey           | 33413    | 33337     | [1]    | [1]     |
| xxx_bindings_id_fkey                    | 33413    | 33405     | [2]    | [1]     |
| yyy_bindings_binding_def_fkey           | 33437    | 33337     | [1]    | [1]     |
| yyy_bindings_id_fkey                    | 33437    | 33405     | [2]    | [1]     |
+-----------------------------------------+----------+-----------+--------+---------+

*/
