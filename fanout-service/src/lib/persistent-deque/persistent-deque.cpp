#include "persistent-deque/persistent-deque.h"

using Element = PersistentDeque::PersistentDeque::Element;

PersistentDeque::PersistentDeque(std::string name)
    : conn_("postgresql", "5432"),  // TODO: parameterize
      name_(name),
      table_("__deque_" + name + "__")
{
    std::stringstream ss;
    ss << "CREATE TABLE IF NOT EXISTS " << table_ << " ("
       << " _seq    BIGSERIAL NOT NULL,"
          " _dir    INT NOT NULL,"
          " _pos    BIGINT GENERATED ALWAYS AS (_seq * _dir) STORED,"
          " _data   JSONB NOT NULL,"
          ""
          " PRIMARY KEY (_pos),"
          " CHECK (_dir IN (-1, 1))"
          ");";

    pqxx::work w(conn_);
    w.exec(ss.str());
    w.commit();
}

PersistentDeque::~PersistentDeque()
{
}

Error PersistentDeque::PushBack(json& data)
{
    // build query with data and forward direction (_dir = 1)
    // i.e., give latest position
    std::stringstream ss;
    ss << "INSERT INTO " << table_ << " (_dir, _data)"
       << " VALUES (1, '" << data.dump() << "'::jsonb);";

    try
    {
        pqxx::work w(conn_);
        w.exec(ss.str());
        w.commit();
    }
    catch (pqxx::sql_error const& e)
    {
        return Error({
            {     "file",                  __FILE__},
            {     "line",  std::to_string(__LINE__)},
            {     "func",                  __func__},
            {    "table",                    table_},
            {  "message", "failed with a sql_error"},
            {"sql_error",                  e.what()}
        });
    }
    catch (std::exception const& e)
    {
        return Error({
            {     "file",                   __FILE__},
            {     "line",   std::to_string(__LINE__)},
            {     "func",                   __func__},
            {    "table",                     table_},
            {  "message", "failed with an exception"},
            {"exception",                   e.what()}
        });
    }

    return NoError;
}

Error PersistentDeque::PushFront(json& data)
{
    // build query with data and reverse direction (_dir = -1)
    // i.e., give earliest position
    std::stringstream ss;
    ss << "INSERT INTO " << table_ << " (_dir, _data)"
       << " VALUES (-1, '" << data.dump() << "'::jsonb);";

    try
    {
        pqxx::work w(conn_);
        w.exec(ss.str());
        w.commit();
    }
    catch (pqxx::sql_error const& e)
    {
        return Error({
            {     "file",                  __FILE__},
            {     "line",  std::to_string(__LINE__)},
            {     "func",                  __func__},
            {    "table",                    table_},
            {  "message", "failed with a sql_error"},
            {"sql_error",                  e.what()}
        });
    }
    catch (std::exception const& e)
    {
        return Error({
            {     "file",                   __FILE__},
            {     "line",   std::to_string(__LINE__)},
            {     "func",                   __func__},
            {    "table",                     table_},
            {  "message", "failed with an exception"},
            {"exception",                   e.what()}
        });
    }

    return NoError;
}

std::pair<Element, Error> PersistentDeque::Back(bool pop)
{
    Element e;
    e.table_ = table_;

    // select the row with latest position
    std::stringstream ss;
    ss << "SELECT _pos, _data FROM " << table_
       << " ORDER BY _pos DESC LIMIT 1"
          " FOR UPDATE SKIP LOCKED;";

    try
    {
        // convert query result to element
        e.w_    = std::make_unique<pqxx::work>(conn_);
        auto r  = e.w_->exec1(ss.str());
        e.pos_  = r["_pos"].as<int64_t>();
        e.data_ = json::parse(r["_data"].c_str());

        // if pop = true, then delete from deque now
        if (pop)
        {
            if (auto err = e.Pop(); err)
            {
                return {Element(), std::move(err)};
            }
        }
    }
    catch (pqxx::sql_error const& e)
    {
        Error err = {
            {     "file",                  __FILE__},
            {     "line",  std::to_string(__LINE__)},
            {     "func",                  __func__},
            {    "table",                    table_},
            {  "message", "failed with a sql_error"},
            {"sql_error",                  e.what()}
        };
        return {Element(), std::move(err)};
    }
    catch (std::exception const& e)
    {
        Error err = {
            {     "file",                   __FILE__},
            {     "line",   std::to_string(__LINE__)},
            {     "func",                   __func__},
            {    "table",                     table_},
            {  "message", "failed with an exception"},
            {"exception",                   e.what()}
        };
        return {Element(), std::move(err)};
    }

    return {std::move(e), NoError};
}

std::pair<Element, Error> PersistentDeque::Front(bool pop)
{
    Element e;
    e.table_ = table_;

    // select the row with earliest position
    std::stringstream ss;
    ss << "SELECT _pos, _data FROM " << table_
       << " ORDER BY _pos ASC LIMIT 1"
          " FOR UPDATE SKIP LOCKED;";

    try
    {
        // convert query result to element
        e.w_    = std::make_unique<pqxx::work>(conn_);
        auto r  = e.w_->exec1(ss.str());
        e.pos_  = r["_pos"].as<int64_t>();
        e.data_ = json::parse(r["_data"].c_str());

        // if pop = true, then delete from deque now
        if (pop)
        {
            if (auto err = e.Pop(); err)
            {
                return {Element(), std::move(err)};
            }
        }
    }
    catch (pqxx::sql_error const& e)
    {
        Error err = {
            {     "file",                  __FILE__},
            {     "line",  std::to_string(__LINE__)},
            {     "func",                  __func__},
            {    "table",                    table_},
            {  "message", "failed with a sql_error"},
            {"sql_error",                  e.what()}
        };
        return {Element(), std::move(err)};
    }
    catch (std::exception const& e)
    {
        Error err = {
            {     "file",                   __FILE__},
            {     "line",   std::to_string(__LINE__)},
            {     "func",                   __func__},
            {    "table",                     table_},
            {  "message", "failed with an exception"},
            {"exception",                   e.what()}
        };
        return {Element(), std::move(err)};
    }

    return {std::move(e), NoError};
}

std::pair<size_t, Error> PersistentDeque::Size()
{
    std::stringstream ss;
    ss << "SELECT count(_pos) FROM " << table_;

    try
    {
        pqxx::work w(conn_);
        pqxx::row  r = w.exec1(ss.str());
        return {r["count"].as<size_t>(), NoError};
    }
    catch (pqxx::sql_error const& e)
    {
        Error err = {
            {     "file",                  __FILE__},
            {     "line",  std::to_string(__LINE__)},
            {     "func",                  __func__},
            {    "table",                    table_},
            {  "message", "failed with a sql_error"},
            {"sql_error",                  e.what()}
        };
        return {0, std::move(err)};
    }
    catch (std::exception const& e)
    {
        Error err = {
            {     "file",                   __FILE__},
            {     "line",   std::to_string(__LINE__)},
            {     "func",                   __func__},
            {    "table",                     table_},
            {  "message", "failed with an exception"},
            {"sql_error",                   e.what()}
        };
        return {0, std::move(err)};
    }
}

Element::Element()
{
}

Element::Element(Element&& e) noexcept
    : w_(std::move(e.w_)),
      table_(std::move(e.table_)),
      pos_(std::move(e.pos_)),
      data_(std::move(e.data_))
{
}

Element::~Element()
{
}

Element& Element::operator=(Element&& other)
{
    w_     = std::move(other.w_);
    table_ = std::move(other.table_);
    pos_   = std::move(other.pos_);
    data_  = std::move(other.data_);
    return *this;
}

json& Element::Data()
{
    return data_;
}

Error Element::Pop()
{
    // not inside a transaction which means it's not from a deque
    if (w_ == nullptr)
    {
        return Error({
            {   "file",                 __FILE__},
            {   "line", std::to_string(__LINE__)},
            {   "func",                 __func__},
            {  "table",                   table_},
            {    "pos",     std::to_string(pos_)},
            {"message",    "transaction missing"}
        });
    }

    try
    {
        // delete the element from the deque
        w_->exec("DELETE FROM " + table_ + " WHERE _pos = " + std::to_string(pos_));
    }
    catch (pqxx::sql_error const& e)
    {
        return Error({
            {     "file",                  __FILE__},
            {     "line",  std::to_string(__LINE__)},
            {     "func",                  __func__},
            {    "table",                    table_},
            {      "pos",      std::to_string(pos_)},
            {  "message", "failed with a sql_error"},
            {"sql_error",                  e.what()}
        });
    }
    catch (std::exception const& e)
    {
        return Error({
            {     "file",                   __FILE__},
            {     "line",   std::to_string(__LINE__)},
            {     "func",                   __func__},
            {    "table",                     table_},
            {      "pos",       std::to_string(pos_)},
            {  "message", "failed with an exception"},
            {"exception",                   e.what()}
        });
    }

    // commit and close the transaction
    w_->commit();
    return NoError;
}

void Element::Release()
{
    if (w_)
    {
        auto w = w_.release();
        delete w;
    }
}
