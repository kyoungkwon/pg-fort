
CREATE TABLE IF NOT EXISTS __deque_simple__ (
    _seq    BIGSERIAL NOT NULL,
    _dir    INT NOT NULL,
    _pos    BIGINT GENERATED ALWAYS AS (_seq * _dir) STORED,
    _data   JSONB NOT NULL,
    
    PRIMARY KEY (_pos),
    CHECK (_dir IN (-1, 1))
);

-- push_back
INSERT INTO __deque_simple__ (_dir, _data)
    VALUES (1, '{"bar": "baz", "balance": 0.01}'::jsonb);

-- push_back
INSERT INTO __deque_simple__ (_dir, _data)
    VALUES (1, '{"bar": "baz", "balance": 1.23}'::jsonb);

-- push_back
INSERT INTO __deque_simple__ (_dir, _data)
    VALUES (1, '{"bar": "baz", "balance": 2.25}'::jsonb);

-- push_front
INSERT INTO __deque_simple__ (_dir,  _data)
    VALUES (-1, '{"bar": "baz", "balance": 3.33}'::jsonb);

-- push_front
INSERT INTO __deque_simple__ (_dir, _data)
    VALUES (-1, '{"bar": "baz", "balance": 4.99}'::jsonb);


/*

front <-----------------------> back
4.99 << 3.33 << 0.01 << 1.23 << 2.25
(-5)    (-4)    (1)      (2)     (3)

*/


-- back
BEGIN;
SELECT * FROM __deque_simple__
    ORDER BY _pos DESC LIMIT 1
    FOR UPDATE SKIP LOCKED;

-- front
BEGIN;
SELECT * FROM __deque_simple__
    ORDER BY _pos ASC LIMIT 1
    FOR UPDATE SKIP LOCKED;


DROP TABLE __deque_simple__;
