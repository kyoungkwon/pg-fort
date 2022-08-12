

-- Sequences

CREATE SEQUENCE public.customer_customer_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.actor_actor_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.category_category_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.film_film_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.address_address_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.city_city_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.country_country_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.inventory_inventory_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.language_language_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.payment_payment_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.rental_rental_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.staff_staff_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


CREATE SEQUENCE public.store_store_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

--


SELECT pg_catalog.setval('public.actor_actor_id_seq', 200, true);


SELECT pg_catalog.setval('public.address_address_id_seq', 605, true);


SELECT pg_catalog.setval('public.category_category_id_seq', 16, true);


SELECT pg_catalog.setval('public.city_city_id_seq', 600, true);


SELECT pg_catalog.setval('public.country_country_id_seq', 109, true);


SELECT pg_catalog.setval('public.customer_customer_id_seq', 599, true);


SELECT pg_catalog.setval('public.film_film_id_seq', 1000, true);


SELECT pg_catalog.setval('public.inventory_inventory_id_seq', 4581, true);


SELECT pg_catalog.setval('public.language_language_id_seq', 6, true);


SELECT pg_catalog.setval('public.payment_payment_id_seq', 32098, true);


SELECT pg_catalog.setval('public.rental_rental_id_seq', 16049, true);


SELECT pg_catalog.setval('public.staff_staff_id_seq', 2, true);


SELECT pg_catalog.setval('public.store_store_id_seq', 2, true);

