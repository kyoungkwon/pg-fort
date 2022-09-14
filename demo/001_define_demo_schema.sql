
--

CREATE TABLE folders (
    id          BIGSERIAL,
    name        TEXT NOT NULL,
    parent_id   BIGINT,

    PRIMARY KEY (id),
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
