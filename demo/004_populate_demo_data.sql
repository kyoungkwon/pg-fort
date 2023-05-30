
INSERT INTO folders (name) VALUES ('root');


-- TODO: delete after fan-out is implemented
BIND ACCESS ROLE doc_viewer TO tom@amzn ON folders (SELECT id FROM folders WHERE name = 'root');


do $$
declare
    root_id BIGINT;
begin
    SELECT id INTO root_id FROM folders WHERE name = 'root';
    INSERT INTO folders (name, parent_id) VALUES ('folder-a', root_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-b', root_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-c', root_id);
    INSERT INTO documents (name, folder_id, tags) VALUES ('readme', root_id, ARRAY['hello', 'world', 'tour']);
end;
$$;


-- TODO: delete after fan-out is implemented
BIND ACCESS ROLE viewer TO sam@amzn ON folders (SELECT id FROM folders WHERE name = 'folder-a');


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-a';
    INSERT INTO folders (name, parent_id) VALUES ('folder-d', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-e', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-f', cur_id);
    INSERT INTO documents (name, folder_id, tags) VALUES ('a-bird', cur_id, ARRAY['world', 'tour', 'guide']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('a-cola', cur_id, ARRAY['tour', 'guide', 'line']);
end;
$$;


-- TODO: delete after fan-out is implemented
BIND ACCESS ROLE editor TO sam@amzn ON folders (SELECT id FROM folders WHERE name = 'folder-d');


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-b';
    INSERT INTO folders (name, parent_id) VALUES ('folder-g', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-h', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-i', cur_id);
    INSERT INTO documents (name, folder_id, tags) VALUES ('b-desk', cur_id, ARRAY['guide', 'line', 'curve']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('b-eraser', cur_id, ARRAY['line', 'curve', 'circle']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-c';
    INSERT INTO folders (name, parent_id) VALUES ('folder-j', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-k', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-l', cur_id);
    INSERT INTO documents (name, folder_id, tags) VALUES ('c-fanta', cur_id, ARRAY['curve', 'circle', 'hello']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('c-gold', cur_id, ARRAY['circle', 'hello', 'world']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-d';
    INSERT INTO folders (name, parent_id) VALUES ('folder-m', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-n', cur_id);
    INSERT INTO folders (name, parent_id) VALUES ('folder-o', cur_id);
    INSERT INTO documents (name, folder_id, tags) VALUES ('d-honey', cur_id, ARRAY['hello', 'world', 'tour']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('d-ink', cur_id, ARRAY['world', 'tour', 'guide']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-e';
    INSERT INTO documents (name, folder_id, tags) VALUES ('e-jelly', cur_id, ARRAY['tour', 'guide', 'line']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('e-kitten', cur_id, ARRAY['guide', 'line', 'curve']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-f';
    INSERT INTO documents (name, folder_id, tags) VALUES ('f-library', cur_id, ARRAY['line', 'curve', 'circle']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('f-milk', cur_id, ARRAY['curve', 'circle', 'hello']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-g';
    INSERT INTO documents (name, folder_id, tags) VALUES ('g-napkin', cur_id, ARRAY['circle', 'hello', 'world']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('g-oyster', cur_id, ARRAY['hello', 'world', 'tour']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-h';
    INSERT INTO documents (name, folder_id, tags) VALUES ('h-parrot', cur_id, ARRAY['world', 'tour', 'guide']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('h-quiz', cur_id, ARRAY['tour', 'guide', 'line']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-i';
    INSERT INTO documents (name, folder_id, tags) VALUES ('i-ribs', cur_id, ARRAY['guide', 'line', 'curve']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('i-steak', cur_id, ARRAY['line', 'curve', 'circle']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-j';
    INSERT INTO documents (name, folder_id, tags) VALUES ('j-tofu', cur_id, ARRAY['curve', 'circle', 'hello']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('j-udon', cur_id, ARRAY['circle', 'hello', 'world']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-k';
    INSERT INTO documents (name, folder_id, tags) VALUES ('k-vanilla', cur_id, ARRAY['hello', 'world', 'tour']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('k-wasabi', cur_id, ARRAY['world', 'tour', 'guide']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-l';
    INSERT INTO documents (name, folder_id, tags) VALUES ('l-xylitol', cur_id, ARRAY['tour', 'guide', 'line']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('l-yogurt', cur_id, ARRAY['guide', 'line', 'curve']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-m';
    INSERT INTO documents (name, folder_id, tags) VALUES ('m-zucchini', cur_id, ARRAY['line', 'curve', 'circle']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('m-apple', cur_id, ARRAY['curve', 'circle', 'hello']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-n';
    INSERT INTO documents (name, folder_id, tags) VALUES ('n-bagel', cur_id, ARRAY['circle', 'hello', 'world']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('n-coffee', cur_id, ARRAY['hello', 'world', 'tour']);
end;
$$;


do $$
declare
    cur_id BIGINT;
begin
    SELECT id INTO cur_id FROM folders WHERE name = 'folder-o';
    INSERT INTO documents (name, folder_id, tags) VALUES ('o-date', cur_id, ARRAY['world', 'tour', 'guide']);
    INSERT INTO documents (name, folder_id, tags) VALUES ('o-egg', cur_id, ARRAY['tour', 'guide', 'line']);
end;
$$;

