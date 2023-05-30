
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
	origin		REGCLASS NOT NULL,
	origin_id	BIGINT NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE inheritances (
	id			BIGSERIAL NOT NULL,
	src 		REGCLASS NOT NULL,
	dst			REGCLASS NOT NULL,
	src_query	TEXT NOT NULL,

	PRIMARY KEY (id),
	UNIQUE (src, dst)	-- for simplicity
);

-- (DEFAULT SELF-MARKER)
INSERT INTO inheritances (id, src, dst, src_query)
	VALUES (0, 'inheritances', 'inheritances', '');

------------------------------------------------------

CREATE TABLE aaa (
	id	BIGSERIAL NOT NULL,
	s	TEXT NOT NULL,
	i	INTEGER NOT NULL,
	b	BOOLEAN NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE aaa_bindings (
	id			BIGINT NOT NULL,
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT NOT NULL DEFAULT 0,
	ts			TIMESTAMP DEFAULT NOW(),

	PRIMARY KEY (id, inheritance, ref),
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
	id			BIGINT NOT NULL,
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT NOT NULL DEFAULT 0,
	ts			TIMESTAMP DEFAULT NOW(),

	PRIMARY KEY (id, inheritance, ref),
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
	id			BIGINT NOT NULL,
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT NOT NULL DEFAULT 0,
	ts			TIMESTAMP DEFAULT NOW(),

	PRIMARY KEY (id, inheritance, ref),
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
	id			BIGINT NOT NULL,
	role		TEXT NOT NULL,
	principal	TEXT NOT NULL,
	ref			BIGINT NOT NULL,
	inheritance	BIGINT NOT NULL DEFAULT 0,
	ts			TIMESTAMP DEFAULT NOW(),

	PRIMARY KEY (id, inheritance, ref),
	FOREIGN KEY (id) REFERENCES ddd (id) ON DELETE CASCADE,
	FOREIGN KEY (ref) REFERENCES binding_refs (id) ON DELETE CASCADE,
	FOREIGN KEY (inheritance) REFERENCES inheritances (id) ON DELETE CASCADE
);

------------------------------------------------------

CREATE OR REPLACE FUNCTION set_bindings() RETURNS TRIGGER AS $$
	DECLARE
    	i RECORD;
	BEGIN
		FOR i IN
			SELECT id, src, dst, src_query
			FROM inheritances
			WHERE dst = TG_TABLE_NAME::REGCLASS
		LOOP
			EXECUTE format(
				$query$
					WITH n AS
						(INSERT INTO %I_bindings (id, role, principal, ref, inheritance, ts)
							SELECT $1.id, role, principal, ref, %s, $2
								FROM %I_bindings
								WHERE id = (%s)
							ON CONFLICT (id, inheritance, ref) DO UPDATE SET ts = $2
							RETURNING *)
					DELETE FROM %1$I_bindings b
						USING n
						WHERE b.id = $1.id
						AND b.inheritance = n.inheritance
						AND b.ref != n.ref;
				$query$,
				i.dst, i.id, i.src, i.src_query) USING NEW, NOW();
		END LOOP;

		RETURN NULL;
	END;
$$ LANGUAGE plpgsql;

------------------------------------------------------

CREATE OR REPLACE TRIGGER aaa_upsert
	AFTER INSERT OR UPDATE ON aaa
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();

CREATE OR REPLACE TRIGGER bbb_upsert
	AFTER INSERT OR UPDATE ON bbb
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();

CREATE OR REPLACE TRIGGER ccc_upsert
	AFTER INSERT OR UPDATE ON ccc
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();

CREATE OR REPLACE TRIGGER ddd_upsert
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

-- BIND ACCESS ROLE viewer TO kim@amzn ON aaa (SELECT id FROM aaa WHERE s = 'first')
WITH r AS
	(INSERT INTO binding_refs (origin, origin_id)
		SELECT 'aaa', id
		FROM aaa
		WHERE s = 'first'
		RETURNING *)
INSERT INTO aaa_bindings (role, principal, id, ref)
	SELECT 'viewer', 'kim@amzn', origin_id, id
	FROM r;

-- BIND ACCESS ROLE viewer TO kim@amzn ON aaa (SELECT id FROM aaa WHERE s = 'second')
WITH r AS
	(INSERT INTO binding_refs (origin, origin_id)
		SELECT 'aaa', id
		FROM aaa
		WHERE s = 'second'
		RETURNING *)
INSERT INTO aaa_bindings (role, principal, id, ref)
	SELECT 'viewer', 'kim@amzn', origin_id, id
	FROM r;

-- BIND ACCESS ROLE staff TO sam@amzn ON aaa (SELECT id FROM aaa WHERE s = 'second')
WITH r AS
	(INSERT INTO binding_refs (origin, origin_id)
		SELECT 'aaa', id
		FROM aaa
		WHERE s = 'second'
		RETURNING *)
INSERT INTO aaa_bindings (role, principal, id, ref)
	SELECT 'staff', 'sam@amzn', origin_id, id
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

UPDATE ccc
	SET aaa_id = (SELECT id from aaa where s = 'second')
	WHERE s = 'seventh';

INSERT INTO ccc (aaa_id, bbb_id, s, i)
	SELECT a.id, b.id, 'eighth', 8
	FROM aaa a, bbb b
	WHERE a.s = 'first' AND b.s = 'fifth';

INSERT INTO ccc (aaa_id, bbb_id, s, i)
	SELECT a.id, b.id, 'ninth', 9
	FROM aaa a, bbb b
	WHERE a.s = 'second' AND b.s = 'fourth';

------------------------------------------------------

-- UNBIND ACCESS ROLE viewer FROM sam@amzn ON aaa (SELECT id FROM aaa WHERE s = 'first')
DELETE FROM binding_refs r
	USING aaa_bindings b
	WHERE b.id = (SELECT id FROM aaa WHERE s = 'first')
	AND b.role = 'viewer'
	AND b.principal = 'sam@amzn'
	AND b.inheritance = 0
	AND b.ref = r.id;

-- UNBIND ACCESS ROLE viewer FROM kim@amzn ON aaa (SELECT id FROM aaa WHERE s = 'second')
DELETE FROM binding_refs r
	USING aaa_bindings b
	WHERE b.id = (SELECT id FROM aaa WHERE s = 'second')
	AND b.role = 'viewer'
	AND b.principal = 'kim@amzn'
	AND b.inheritance = 0
	AND b.ref = r.id;

------------------------------------------------------

-- DELETE ACCESS INHERITANCE FROM bbb TO ccc;
DELETE FROM inheritances
	WHERE src = 'bbb'::REGCLASS
	AND dst = 'ccc'::REGCLASS;

------------------------------------------------------
