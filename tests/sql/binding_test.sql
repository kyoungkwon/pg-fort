
------------------------------------------------------

DROP TABLE binding_defs, xxx, xxx_bindings, yyy;

CREATE TABLE binding_defs (
	id			BIGSERIAL NOT NULL,
	relation	TEXT NOT NULL,
	condition	TEXT NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE xxx (
	id	BIGSERIAL NOT NULL,
	s	TEXT NOT NULL,
	i	INTEGER NOT NULL,
	b	BOOLEAN NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE xxx_bindings (
	binding_def	BIGINT NOT NULL,
	id			BIGINT NOT NULL,

	PRIMARY KEY (binding_def, id),
	FOREIGN KEY (binding_def) REFERENCES binding_defs (id) ON DELETE CASCADE,
	FOREIGN KEY (id) REFERENCES xxx (id) ON DELETE CASCADE
);

CREATE TABLE yyy (
	id	BIGSERIAL NOT NULL,
	s	TEXT NOT NULL,
	i	INTEGER NOT NULL,
	b	BOOLEAN NOT NULL,

	PRIMARY KEY (id)
);

CREATE TABLE yyy_bindings (
	binding_def	BIGINT NOT NULL,
	id			BIGINT NOT NULL,

	PRIMARY KEY (binding_def, id),
	FOREIGN KEY (binding_def) REFERENCES binding_defs (id) ON DELETE CASCADE,
	FOREIGN KEY (id) REFERENCES xxx (id) ON DELETE CASCADE
);

CREATE OR REPLACE FUNCTION create_bindings() RETURNS TRIGGER AS $$
	BEGIN
		EXECUTE format(
			$query$
				INSERT INTO %I_bindings (binding_def, id)
					SELECT $1.id, id
					FROM %1$I
					WHERE %s
			$query$,
			NEW.relation,
			NEW.condition) USING NEW;
		RETURN NULL;
	END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION evaluate(data record, condition text) RETURNS BOOLEAN
	STABLE
	STRICT
	PARALLEL SAFE
	AS $$
	DECLARE
    	result BOOLEAN;
	BEGIN
    	EXECUTE format(
			$query$
				SELECT $1.%s
			$query$,
			condition) INTO result USING data;
    	return result;
	END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION set_bindings() RETURNS TRIGGER AS $$
	BEGIN
		-- set direct bindings
		EXECUTE format(
			$query$
				INSERT INTO %I_bindings (binding_def, id)
					SELECT DISTINCT id, $1.id
					FROM binding_defs
					WHERE relation = %1$L
					AND evaluate($1, condition)
			$query$,
			TG_TABLE_NAME) USING NEW;

		-- set inherited bindings
		-- TODO
		
		RETURN NULL;
	END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER binding_def_insert
    AFTER INSERT ON binding_defs
    FOR EACH ROW
	EXECUTE FUNCTION create_bindings();

CREATE OR REPLACE TRIGGER xxx_insert
	AFTER INSERT ON xxx
	FOR EACH ROW
	EXECUTE FUNCTION set_bindings();


INSERT INTO xxx (s, i, b) VALUES ('first', 1, true);
INSERT INTO xxx (s, i, b) VALUES ('second', 2, false);
INSERT INTO xxx (s, i, b) VALUES ('third', 3, true);
INSERT INTO xxx (s, i, b) VALUES ('fourth', 4, false);
INSERT INTO xxx (s, i, b) VALUES ('fifth', 5, true);
INSERT INTO xxx (s, i, b) VALUES ('sixth', 6, false);
INSERT INTO xxx (s, i, b) VALUES ('seventh', 7, true);
INSERT INTO xxx (s, i, b) VALUES ('eighth', 8, false);
INSERT INTO xxx (s, i, b) VALUES ('ninth', 9, true);
INSERT INTO xxx (s, i, b) VALUES ('tenth', 10, false);

INSERT INTO binding_defs (relation, condition) VALUES ('xxx', 'i > 5');
INSERT INTO binding_defs (relation, condition) VALUES ('xxx', 'b = true');
INSERT INTO binding_defs (relation, condition) VALUES ('yyy', 'i > 5');

INSERT INTO xxx (s, i, b) VALUES ('miracle', 100, true);

------------------------------------------------------
