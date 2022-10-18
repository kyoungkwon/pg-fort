
-- create user-data schema
CREATE TABLE folders (
    id          BIGSERIAL,
    name        TEXT NOT NULL,
    parent_id   BIGINT,

    PRIMARY KEY (id),
    FOREIGN KEY (parent_id) REFERENCES folders (id) ON DELETE RESTRICT,
    UNIQUE (parent_id, name)
);

CREATE TABLE documents (
    id          BIGSERIAL,
    name        TEXT NOT NULL,
    tags        TEXT[],
    folder_id   BIGINT,

    PRIMARY KEY (id),
    FOREIGN KEY (folder_id) REFERENCES folders (id) ON DELETE RESTRICT,
    UNIQUE (folder_id, name)
);

CREATE INDEX folders_tags_idx ON documents USING GIN (tags);


-- ENABLE ACCESS CONTROL folders
do $$
begin
    EXECUTE format(
        $cmds$
            CREATE TABLE %1$I__access_bindings__ (
                id			BIGINT NOT NULL,
                role		TEXT NOT NULL,
                principal	TEXT NOT NULL,
                ref			BIGINT NOT NULL,
                inheritance	BIGINT NOT NULL DEFAULT 0,
                ts			TIMESTAMP DEFAULT NOW(),

                PRIMARY KEY (id, inheritance, ref),
                FOREIGN KEY (id) REFERENCES %1$I (id) ON DELETE CASCADE,
                FOREIGN KEY (role) REFERENCES __access_roles__ (name) ON DELETE RESTRICT,
                FOREIGN KEY (ref) REFERENCES __access_binding_refs__ (id) ON DELETE CASCADE,
                FOREIGN KEY (inheritance) REFERENCES __access_inheritances__ (id) ON DELETE CASCADE
            );

            CREATE TRIGGER %1$I__upsert__
                AFTER INSERT OR UPDATE ON %1$I
                FOR EACH ROW
                EXECUTE FUNCTION __set_access_bindings__();

            CREATE VIEW %1$I__acls__ AS
                SELECT b.id, b.role, b.principal, r.operation, r.columns
                FROM %1$I__access_bindings__ b, __access_roles_expanded__ r
                WHERE r.relation = %1$L::REGCLASS AND b.role = r.name;
        $cmds$,
        'folders');
end;
$$;


-- ENABLE ACCESS CONTROL documents
do $$
begin
    EXECUTE format(
        $cmds$
            CREATE TABLE %1$I__access_bindings__ (
                id			BIGINT NOT NULL,
                role		TEXT NOT NULL,
                principal	TEXT NOT NULL,
                ref			BIGINT NOT NULL,
                inheritance	BIGINT NOT NULL DEFAULT 0,
                ts			TIMESTAMP DEFAULT NOW(),

                PRIMARY KEY (id, inheritance, ref),
                FOREIGN KEY (id) REFERENCES %1$I (id) ON DELETE CASCADE,
                FOREIGN KEY (role) REFERENCES __access_roles__ (name) ON DELETE RESTRICT,
                FOREIGN KEY (ref) REFERENCES __access_binding_refs__ (id) ON DELETE CASCADE,
                FOREIGN KEY (inheritance) REFERENCES __access_inheritances__ (id) ON DELETE CASCADE
            );

            CREATE TRIGGER %1$I__upsert__
                AFTER INSERT OR UPDATE ON %1$I
                FOR EACH ROW
                EXECUTE FUNCTION __set_access_bindings__();

            CREATE VIEW %1$I__acls__ AS
                SELECT b.id, b.role, b.principal, r.operation, r.columns
                FROM %1$I__access_bindings__ b, __access_roles_expanded__ r
                WHERE r.relation = %1$L::REGCLASS AND b.role = r.name;
        $cmds$,
        'documents');
end;
$$;
