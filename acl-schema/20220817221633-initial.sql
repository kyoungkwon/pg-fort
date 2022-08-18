-- +migrate Up

CREATE TABLE __access_permissions__ (
	id				BIGSERIAL NOT NULL,
	name			TEXT NOT NULL,
	table_name		TEXT NOT NULL,
	operation		TEXT NOT NULL,
	column_names	TEXT[],

	PRIMARY KEY (name)
);

CREATE TABLE __access_roles__ (
	id			BIGSERIAL NOT NULL,
	name		TEXT NOT NULL,
	permissions	TEXT[] NOT NULL,

	PRIMARY KEY (name)
);

CREATE TABLE __access_bindings__ (
	id			BIGSERIAL NOT NULL,
	table_name	TEXT NOT NULL,
	condition	TEXT NOT NULL,
	principal	TEXT NOT NULL,
	alias		TEXT,

	PRIMARY KEY (id),
	UNIQUE (alias)
);

CREATE TABLE __access_inheritances__ (
	id			BIGSERIAL NOT NULL,
	source		TEXT NOT NULL,
	destination TEXT NOT NULL,
	condition	TEXT NOT NULL,

	PRIMARY KEY (id)
);


-- +migrate Down

DROP TABLE IF EXISTS __access_inheritances__;
DROP TABLE IF EXISTS __access_bindings__;
DROP TABLE IF EXISTS __access_roles__;
DROP TABLE IF EXISTS __access_permissions__;
