

-- something like.... row security policy?
-- how do I express inheritance?


------- WRONG WAY TO THINK: using database itself as identity store
-- eg) only customer who rented can see their rental records
-- eg) only the staff who checked out can see the rental records
-- eg) the store manager can see the rental records


------- RIGHT WAY TO THINK: identity store (directory) is completely external
-- eg) auth token should tell identity and use the identity to check policy



---




------- WRONG WAY TO THINK: this table is where policies are managed
------- RIGHT WAY TO THINK: this table stores fully-computed-and-flattened and join-ready permission-to-identity mappings


-- TODO: how to pass auth token data to postgresql?
-- 


-- use join to filter out unauthorized records
CREATE ACCESS PERMISSION rental_veiw ON rental FOR SELECT;


-- use before trigger to perform access check?
CREATE ACCESS PERMISSION rental_edit ON rental FOR UPDATE;


-- more niche use case
CREATE ACCESS PERMISSION rental_extend ON rental FOR UPDATE OF return_date;


-- use before trigger to perform access check?
CREATE ACCESS PERMISSION rental_delete ON rental FOR DELETE;


-- can create object but will trace back inheritance to perform access check
--    eg) INSERT INTO rental (rental_id, inventory_id) VALUES (20, 399);  (O)
--        INSERT INTO rental (rental_id, inventory_id) VALUES (20, 400);  (X)
CREATE ACCESS PERMISSION rental_create ON rental FOR INSERT;


-- a clerk cannot delete rental records
CREATE ACCESS ROLE clerk
    WITH rental_veiw
         rental_edit
         rental_create;


-- define store -> inventory inheritance
CREATE ACCESS INHERITANCE
    FROM store(store_id)
    TO inventory(store_id);


-- define inventory -> rental inheritance
CREATE ACCESS INHERITANCE
    FROM inventory(inventory_id)
    TO rental(inventory_id);


-- set role binding on a store whose ACLs will propagate to rentals via inheritance
CREATE ACCESS BINDING bellevue_clerk
    ROLE clerk
    ON store
    WHERE store_id = 11 -- Bellevue location
    WITH g:bellevue_staffs; -- group from directory


-- another example
CREATE ACCESS BINDING seattle_clerk
    ROLE clerk
    ON store
    WHERE store_id = 10 -- Seattle location
    WITH g:seattle_staffs; -- group from directory



-- TODO: how to set default role bindings? is this necessary?
--   eg) creator should have some permission?


-- TODO: how will nested queries be handled?
/*
WITH upd AS (
  UPDATE employees SET sales_count = sales_count + 1 WHERE id =
    (SELECT sales_person FROM accounts WHERE name = 'Acme Corporation')
    RETURNING *
)
INSERT INTO employees_log SELECT *, current_timestamp FROM upd;
*/
/*

-- TODO: how will partitioned tables be handled?

*/


-- precomputed table hidden from users
CREATE TABLE public.rental_acl (
    rental_id integer NOT NULL, -- foreign key to rental.rental_id
    permission text NOT NULL,
    identity text NOT NULL
);





-- tables

CREATE TABLE rental (
    rental_id integer DEFAULT nextval('rental_id_seq'::regclass) NOT NULL,
    rental_date timestamp without time zone NOT NULL,
    inventory_id integer NOT NULL,
    customer_id smallint NOT NULL,
    return_date timestamp without time zone,
    staff_id smallint NOT NULL,
    last_update timestamp without time zone DEFAULT now() NOT NULL,

    PRIMARY KEY (rental_id),

    FOREIGN KEY (inventory_id) REFERENCES inventory(inventory_id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT
);

CREATE TABLE inventory (
    inventory_id integer DEFAULT nextval('inventory_id_seq'::regclass) NOT NULL,
    film_id smallint NOT NULL,
    store_id smallint NOT NULL,
    last_update timestamp without time zone DEFAULT now() NOT NULL,

    PRIMARY KEY (inventory_id),

    FOREIGN KEY (film_id) REFERENCES film(film_id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT,

    FOREIGN KEY (store_id) REFERENCES store(store_id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT
);

CREATE TABLE store (
    store_id integer DEFAULT nextval('store_id_seq'::regclass) NOT NULL,
    manager_staff_id smallint NOT NULL,
    address_id smallint NOT NULL,
    last_update timestamp without time zone DEFAULT now() NOT NULL,

    PRIMARY KEY (store_id),

    FOREIGN KEY (address_id) REFERENCES address(address_id)
        ON UPDATE CASCADE
        ON DELETE RESTRICT
);



-- additions


CREATE INDEX idx_fk_inventory_id
    ON public.rental USING btree (inventory_id);


CREATE TRIGGER last_updated
    BEFORE UPDATE ON public.rental
    FOR EACH ROW
        EXECUTE PROCEDURE public.last_updated();



