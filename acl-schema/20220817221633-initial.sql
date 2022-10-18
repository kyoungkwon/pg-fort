-- +migrate Up

CREATE TABLE __access_permissions__ (
	id			BIGSERIAL NOT NULL UNIQUE,
	name		TEXT NOT NULL,
	relation	REGCLASS NOT NULL,
	operation	TEXT NOT NULL,
	columns		TEXT[],

	PRIMARY KEY (name),
	UNIQUE (relation, operation, columns)
);

CREATE TABLE __access_roles__ (
	id			BIGSERIAL NOT NULL UNIQUE,
	name		TEXT NOT NULL,
	permissions	TEXT[] NOT NULL,

	PRIMARY KEY (name)
);

CREATE TABLE __access_roles_denorm__ (
	name		TEXT NOT NULL,
	permission	TEXT NOT NULL,

	PRIMARY KEY (name, permission),
	FOREIGN KEY (name) REFERENCES __access_roles__ (name) ON DELETE CASCADE,
	FOREIGN KEY (permission) REFERENCES __access_permissions__ (name) ON DELETE RESTRICT 
);

CREATE TABLE __access_binding_refs__ (
	id			BIGSERIAL NOT NULL,
	origin		REGCLASS NOT NULL,
	origin_id	BIGINT NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE __access_inheritances__ (
	id			BIGSERIAL NOT NULL,
	src			REGCLASS NOT NULL,
	dst			REGCLASS NOT NULL,
	src_query	TEXT NOT NULL,

	PRIMARY KEY (id),
	UNIQUE (src, dst)
);


-- Speical reference for self-originating bindings
INSERT INTO __access_inheritances__ (id, src, dst, src_query)
	VALUES (0, '__access_inheritances__', '__access_inheritances__', '');


-- TODO: maybe MATERIALIZED VIEW is a better choice
--       perform REFRESH MATERIALIZED VIEW whenever
--         1) any permission changes (UPDATE)
--         2) any role changes (INSERT/UPDATE/DELETE)
--       this will also provide indexes for even more effective joins
CREATE VIEW __access_roles_expanded__ AS
	SELECT r.name, p.relation, p.operation, p.columns
	FROM __access_roles_denorm__ r
	JOIN __access_permissions__ p
	ON r.permission = p.name;


-- +migrate StatementBegin
CREATE FUNCTION __set_access_bindings__() RETURNS TRIGGER AS $$
	DECLARE
    	i RECORD;
	BEGIN
		FOR i IN
			SELECT id, src, dst, src_query
			FROM __access_inheritances__
			WHERE dst = TG_TABLE_NAME::REGCLASS
		LOOP
			EXECUTE format(
				$query$
					WITH n AS
						(INSERT INTO %I__access_bindings__ (id, role, principal, ref, inheritance, ts)
							SELECT $1.id, role, principal, ref, %s, $2
								FROM %I__access_bindings__
								WHERE id = (%s)
							ON CONFLICT (id, inheritance, ref) DO UPDATE SET ts = $2
							RETURNING *)
					DELETE FROM %1$I__access_bindings__ b
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
-- +migrate StatementEnd



-- +migrate Down

DROP FUNCTION IF EXISTS __set_access_bindings__;
DROP VIEW IF EXISTS __access_roles_expanded__;
DROP TABLE IF EXISTS __access_inheritances__;
DROP TABLE IF EXISTS __access_binding_refs__;
DROP TABLE IF EXISTS __access_roles_denorm__;
DROP TABLE IF EXISTS __access_roles__;
DROP TABLE IF EXISTS __access_permissions__;
