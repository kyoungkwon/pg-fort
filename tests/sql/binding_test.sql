
------------------------------------------------------

DROP TABLE IF EXISTS 
	binding_refs,
	inheritances,
	logs,
	aaa,
	aaa_bindings,
	bbb,
	bbb_bindings,
	ccc,
	ccc_bindings,
	ddd,
	ddd_bindings;


CREATE TABLE binding_refs (
	id			BIGSERIAL NOT NULL,
	origin		TEXT NOT NULL,
	origin_id	BIGINT NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE inheritances (
	id			BIGSERIAL NOT NULL,
	src 		TEXT NOT NULL,
	dst			TEXT NOT NULL,
	src_query	TEXT NOT NULL,

	PRIMARY KEY (id),
	UNIQUE(src, dst)	-- for simplicity
);

CREATE TABLE logs (
	id	BIGSERIAL NOT NULL,
	msg	TEXT NOT NULL,

	PRIMARY KEY (id)
);

------------------------------------------------------

CREATE TABLE aaa (
	id	BIGSERIAL NOT NULL,
	s	TEXT NOT NULL,
	i	INTEGER NOT NULL,
	b	BOOLEAN NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE aaa_bindings (
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	id			BIGINT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT,

	PRIMARY KEY (role, principal, id),
	FOREIGN KEY (id) REFERENCES aaa (id) ON DELETE CASCADE,
	FOREIGN KEY (ref) REFERENCES binding_refs (id) ON DELETE CASCADE,
	FOREIGN KEY (inheritance) REFERENCES inheritances (id) ON DELETE CASCADE
);

CREATE TABLE bbb (
	id	BIGSERIAL NOT NULL,
	s	TEXT NOT NULL,
	i	INTEGER NOT NULL,
	b	BOOLEAN NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE bbb_bindings (
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	id			BIGINT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT,

	PRIMARY KEY (role, principal, id),
	FOREIGN KEY (id) REFERENCES bbb (id) ON DELETE CASCADE,
	FOREIGN KEY (ref) REFERENCES binding_refs (id) ON DELETE CASCADE,
	FOREIGN KEY (inheritance) REFERENCES inheritances (id) ON DELETE CASCADE
);

CREATE TABLE ccc (
	id		BIGSERIAL NOT NULL,
	aaa_id	BIGINT NOT NULL,
	bbb_id	BIGINT NOT NULL,
	s		TEXT NOT NULL,
	i		INTEGER NOT NULL,
	b		BOOLEAN NOT NULL DEFAULT FALSE,

	PRIMARY KEY (s, i),
	UNIQUE (id),
	FOREIGN KEY (aaa_id) REFERENCES aaa (id) ON DELETE RESTRICT,
	FOREIGN KEY (bbb_id) REFERENCES bbb (id) ON DELETE RESTRICT
);

CREATE TABLE ccc_bindings (
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	id			BIGINT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT,

	PRIMARY KEY (role, principal, id),
	FOREIGN KEY (id) REFERENCES ccc (id) ON DELETE CASCADE,
	FOREIGN KEY (ref) REFERENCES binding_refs (id) ON DELETE CASCADE,
	FOREIGN KEY (inheritance) REFERENCES inheritances (id) ON DELETE CASCADE
);

CREATE TABLE ddd (
	id		BIGSERIAL NOT NULL,
	ccc_s	TEXT NOT NULL,
	ccc_i	INTEGER NOT NULL,
	s		TEXT NOT NULL,
	i		INTEGER NOT NULL,
	b		BOOLEAN NOT NULL DEFAULT FALSE,

	PRIMARY KEY (id),
	FOREIGN KEY (ccc_s, ccc_i) REFERENCES ccc (s, i) ON DELETE RESTRICT
);

CREATE TABLE ddd_bindings (
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	id			BIGINT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT,

	PRIMARY KEY (role, principal, id),
	FOREIGN KEY (id) REFERENCES ddd (id) ON DELETE CASCADE,
	FOREIGN KEY (ref) REFERENCES binding_refs (id) ON DELETE CASCADE,
	FOREIGN KEY (inheritance) REFERENCES inheritances (id) ON DELETE CASCADE
);

------------------------------------------------------

DROP FUNCTION IF EXISTS set_bindings;

CREATE OR REPLACE FUNCTION set_bindings() RETURNS TRIGGER AS $$
	DECLARE
    	i RECORD;
	BEGIN
		FOR i IN
			SELECT id, src, dst, src_query
			FROM inheritances
			WHERE dst = TG_TABLE_NAME
		LOOP
			EXECUTE format(
				$query$
					INSERT INTO %I_bindings (role, principal, id, ref, inheritance)
						SELECT role, principal, $1.id, ref, %L
						FROM %I_bindings
						WHERE id = (%s)
				$query$,
				i.dst, i.id, i.src, i.src_query) USING NEW;
		END LOOP;

		RETURN NULL;
	END;
$$ LANGUAGE plpgsql;

------------------------------------------------------

CREATE OR REPLACE TRIGGER aaa_insert
	AFTER INSERT ON aaa
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();

CREATE OR REPLACE TRIGGER bbb_insert
	AFTER INSERT ON bbb
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();

CREATE OR REPLACE TRIGGER ccc_insert
	AFTER INSERT ON ccc
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();

CREATE OR REPLACE TRIGGER ddd_insert
	AFTER INSERT ON ddd
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();

------------------------------------------------------

-- CREATE ACCESS INHERITANCE FROM aaa (id) TO ccc (aaa_id);
INSERT INTO inheritances (src, dst, src_query)
	VALUES ('aaa', 'ccc', 'SELECT id FROM aaa WHERE id = $1.aaa_id');

-- CREATE ACCESS INHERITANCE FROM bbb (id) TO ccc (bbb_id);
INSERT INTO inheritances (src, dst, src_query)
	VALUES ('bbb', 'ccc', 'SELECT id FROM bbb WHERE id = $1.bbb_id');

-- CREATE ACCESS INHERITANCE FROM ccc (s, i) TO ddd (ccc_s, ccc_i);
INSERT INTO inheritances (src, dst, src_query)
	VALUES ('ccc', 'ddd', 'SELECT id FROM bbb WHERE s = $1.ccc_s AND i = $1.ccc_i');

------------------------------------------------------

INSERT INTO aaa (s, i, b) VALUES ('first', 1, true);
INSERT INTO aaa (s, i, b) VALUES ('second', 2, true);
INSERT INTO aaa (s, i, b) VALUES ('third', 3, true);

INSERT INTO bbb (s, i, b) VALUES ('fourth', 4, true);
INSERT INTO bbb (s, i, b) VALUES ('fifth', 5, true);
INSERT INTO bbb (s, i, b) VALUES ('sixth', 6, true);

------------------------------------------------------

-- BIND ACCESS ROLE viewer TO sam@amzn ON aaa (SELECT id FROM aaa WHERE s = 'first')
WITH r AS
	(INSERT INTO binding_refs (origin, origin_id)
		SELECT 'aaa', id
		FROM aaa
		WHERE s = 'first'
		RETURNING *)
INSERT INTO aaa_bindings (role, principal, id, ref)
	SELECT 'viewer', 'sam@amzn', origin_id, id
	FROM r;

-- BIND ACCESS ROLE editor TO tom@amzn ON bbb (SELECT id FROM bbb WHERE s = 'fourth')
WITH r AS
	(INSERT INTO binding_refs (origin, origin_id)
		SELECT 'bbb', id
		FROM bbb
		WHERE s = 'fourth'
		RETURNING *)
INSERT INTO bbb_bindings (role, principal, id, ref)
	SELECT 'editor', 'tom@amzn', origin_id, id
	FROM r;

------------------------------------------------------

INSERT INTO ccc (aaa_id, bbb_id, s, i)
	SELECT a.id, b.id, 'seventh', 7
	FROM aaa a, bbb b
	WHERE a.s = 'first' AND b.s = 'fourth';

INSERT INTO ccc (aaa_id, bbb_id, s, i)
	SELECT a.id, b.id, 'eighth', 8
	FROM aaa a, bbb b
	WHERE a.s = 'first' AND b.s = 'fifth';

INSERT INTO ccc (aaa_id, bbb_id, s, i)
	SELECT a.id, b.id, 'ninth', 9
	FROM aaa a, bbb b
	WHERE a.s = 'second' AND b.s = 'fourth';

------------------------------------------------------

-- UNBIND ACCESS ROLE viewer TO sam@amzn ON aaa (SELECT id FROM aaa WHERE s = 'first')
DELETE FROM binding_refs r
	USING aaa_bindings b
	WHERE b.id = (SELECT id FROM aaa WHERE s = 'first')
	AND b.inheritance IS NULL
	AND b.ref = r.id;

------------------------------------------------------

-- DELETE ACCESS INHERITANCE FROM bbb TO ccc;
DELETE FROM inheritances
	WHERE src = 'bbb'
	AND dst = 'ccc';
