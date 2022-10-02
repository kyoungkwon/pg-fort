-- +migrate Up

CREATE TABLE __access_permissions__ (
	id			BIGSERIAL NOT NULL UNIQUE,
	name		TEXT NOT NULL,
	relation	TEXT NOT NULL,
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

CREATE TABLE __access_binding_defs__ (
	id			BIGSERIAL NOT NULL,
	role		TEXT NOT NULL,
	relation	TEXT NOT NULL,
	condition	TEXT NOT NULL,
	principal	TEXT NOT NULL,
	alias		TEXT,

	PRIMARY KEY (id),
	FOREIGN KEY (role) REFERENCES __access_roles__ (name) ON DELETE RESTRICT,
	UNIQUE (role, relation, condition, principal),
	UNIQUE (alias)
);

CREATE TABLE __access_inheritances__ (
	id			BIGSERIAL NOT NULL,
	source		TEXT NOT NULL,
	destination TEXT NOT NULL,
	condition	TEXT NOT NULL,

	PRIMARY KEY (id),
	UNIQUE(source, destination, condition)
);

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

CREATE FUNCTION __create_access_bindings__() RETURNS TRIGGER AS $$
	BEGIN
		EXECUTE format(
			$query$
				INSERT INTO %I__access_bindings__ (binding_def, id)
					SELECT $1.id, id
					FROM %1$I
					WHERE %s
			$query$,
			NEW.relation,
			NEW.condition) USING NEW;
		RETURN NULL;
	END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER __access_binding_def_insert__
    AFTER INSERT ON __access_binding_defs__
    FOR EACH ROW
	EXECUTE FUNCTION __create_access_bindings__();


-- +migrate Down

DROP TRIGGER IF EXISTS __access_binding_def_insert__;
DROP FUNCTION IF EXISTS __create_access_bindings__;
DROP VIEW IF EXISTS __access_roles_expanded__;
DROP TABLE IF EXISTS __access_inheritances__;
DROP TABLE IF EXISTS __access_binding_defs__;
DROP TABLE IF EXISTS __access_roles_denorm__;
DROP TABLE IF EXISTS __access_roles__;
DROP TABLE IF EXISTS __access_permissions__;
