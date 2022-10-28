
CREATE ROLE rlstester;

ALTER TABLE folders ENABLE ROW LEVEL SECURITY;

CREATE POLICY sel_test ON folders TO rlstester USING (id < 10);

SET ROLE rlstester;

SELECT * FROM folders;

do
$$
declare
    f RECORD;
begin
    for f in SELECT * FROM folders
    loop
        raise notice '% - % ', f.id, f.name;
    end loop;
end;
$$;
