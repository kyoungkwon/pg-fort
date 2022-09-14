-- +migrate Up

CREATE TABLE __access_permissions__ (
	id			BIGSERIAL NOT NULL UNIQUE,
	name		TEXT NOT NULL,
	relation	TEXT NOT NULL,
	operation	TEXT NOT NULL,
	columns		TEXT[],

	PRIMARY KEY (name)
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

CREATE TABLE __access_bindings__ (
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


-- +migrate Down

DROP TABLE IF EXISTS __access_inheritances__;
DROP TABLE IF EXISTS __access_bindings__;
DROP TABLE IF EXISTS __access_roles_denorm__;
DROP TABLE IF EXISTS __access_roles__;
DROP TABLE IF EXISTS __access_permissions__;
