-- +migrate Up

CREATE TABLE __access_permissions__ (
	_id			BIGSERIAL NOT NULL UNIQUE,
	_name		TEXT NOT NULL,
	_relation	REGCLASS NOT NULL,
	_operation	TEXT NOT NULL,
	_columns		TEXT[],

	PRIMARY KEY (_name),
	UNIQUE (_relation, _operation, _columns),
	CHECK (_operation IN ('SELECT', 'INSERT', 'UPDATE', 'DELETE', 'ALL'))
);

CREATE TABLE __access_roles__ (
	_id				BIGSERIAL NOT NULL UNIQUE,
	_name			TEXT NOT NULL,
	_permissions	TEXT[] NOT NULL,

	PRIMARY KEY (_name)
);

CREATE TABLE __access_roles_denorm__ (
	_name		TEXT NOT NULL,
	_permission	TEXT NOT NULL,

	PRIMARY KEY (_name, _permission),
	FOREIGN KEY (_name) REFERENCES __access_roles__ (_name) ON DELETE CASCADE,
	FOREIGN KEY (_permission) REFERENCES __access_permissions__ (_name) ON DELETE RESTRICT 
);

CREATE TABLE __access_binding_refs__ (
	_id			BIGSERIAL NOT NULL,
	_origin		REGCLASS NOT NULL,
	_origin_id	BIGINT NOT NULL,

	PRIMARY KEY (_id)
);

CREATE TABLE __access_inheritances__ (
	_id			BIGSERIAL NOT NULL,
	_src		REGCLASS NOT NULL,
	_dst		REGCLASS NOT NULL,
	_src_query	TEXT NOT NULL,

	PRIMARY KEY (_id),
	UNIQUE (_src, _dst)
);


-- Speical reference for self-originating bindings
INSERT INTO __access_inheritances__ (_id, _src, _dst, _src_query)
	VALUES (0, '__access_inheritances__', '__access_inheritances__', '');
	
CREATE RULE __protect_self_inheritance_ref__ AS
	ON DELETE
	TO __access_inheritances__
	WHERE _id = 0
	DO INSTEAD NOTHING;


-- TODO: maybe MATERIALIZED VIEW is a better choice
--       perform REFRESH MATERIALIZED VIEW whenever
--         1) any permission changes (UPDATE)
--         2) any role changes (INSERT/UPDATE/DELETE)
--       this will also provide indexes for even more effective joins
CREATE VIEW __access_roles_expanded__ AS
	SELECT r._name, p._relation, p._operation, p._columns
	FROM __access_roles_denorm__ r
	JOIN __access_permissions__ p
	ON r._permission = p._name;


-- +migrate StatementBegin
CREATE FUNCTION __set_access_bindings__() RETURNS TRIGGER AS $$
	DECLARE
    	i RECORD;
	BEGIN
		FOR i IN
			SELECT _id, _src, _dst, _src_query
			FROM __access_inheritances__
			WHERE _dst = TG_TABLE_NAME::REGCLASS
		LOOP
			EXECUTE format(
				$query$
					WITH n AS
						(INSERT INTO %I__access_bindings__ (_id, _role, _principal, _ref, _inheritance, _ts)
							SELECT $1.id, _role, _principal, _ref, %s, $2
								FROM %I__access_bindings__
								WHERE _id = (%s)
							ON CONFLICT (_id, _inheritance, _ref) DO UPDATE SET _ts = $2
							RETURNING *)
					DELETE FROM %1$I__access_bindings__ b
						USING n
						WHERE b._id = $1.id
						AND b._inheritance = n._inheritance
						AND b._ref != n._ref;
				$query$,
				i._dst, i._id, i._src, i._src_query) USING NEW, NOW();
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
