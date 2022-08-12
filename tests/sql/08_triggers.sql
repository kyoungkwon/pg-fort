

-- Triggers

CREATE TRIGGER film_fulltext_trigger BEFORE INSERT OR UPDATE ON public.film FOR EACH ROW EXECUTE PROCEDURE tsvector_update_trigger('fulltext', 'pg_catalog.english', 'title', 'description');


CREATE TRIGGER last_updated BEFORE UPDATE ON public.actor FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.address FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.category FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.city FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.country FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.customer FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.film FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.film_actor FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.film_category FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.inventory FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.language FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.rental FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.staff FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


CREATE TRIGGER last_updated BEFORE UPDATE ON public.store FOR EACH ROW EXECUTE PROCEDURE public.last_updated();


-- TODO: Add listeners.

