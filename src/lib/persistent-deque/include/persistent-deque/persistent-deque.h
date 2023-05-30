#ifndef __FANOUT_SERVICE_PERSISTENTDEQUE_H__
#define __FANOUT_SERVICE_PERSISTENTDEQUE_H__

#include <iostream>
#include <nlohmann/json.hpp>

#include "common/error.h"
#include "conn/pqxx-conn.h"

using json = nlohmann::json;

// IMPORTANT:
// Single instance is NOT thread-safe. You can open multiple instances and
// use one instance per thread if you want to access a deque concurrently.
class PersistentDeque
{
private:
    PqxxConn    conn_;
    std::string name_;
    std::string table_;

public:
    // TODO: either pass in endpoint/username/password or conn
    PersistentDeque(std::string name);  // creates or opens the queue
    ~PersistentDeque();

    class Element
    {
        friend PersistentDeque;

    private:
        std::unique_ptr<pqxx::work> w_;
        std::string                 table_;
        int64_t                     pos_;
        json                        data_;

        Element();

    public:
        Element(Element&& e) noexcept;
        ~Element();

        Element& operator=(Element&& other);

        json& Data();
        Error Pop();
        void  Release();
    };

    // write functions
    Error PushBack(json& data);
    Error PushFront(json& data);

    // read functions
    // IMPORTANT:
    // There are two different ways to consume deque
    // (1) Back(false) / Front(false) --> Pop():
    //   The caller will exclusively hold an element without deleting
    //   it until Pop() is called. It's useful in case the processing
    //   of the element abruptly terminates before completion, the
    //   element will be automatically released so other callers can
    //   process it. This is the default behavior.
    // (2) Back(true) / Front(true):
    //   The element is fetched and immediately deleted from the deque.
    //   It is up to the caller how to handle the case in which the
    //   processing of the element fails for whatever reason.
    std::pair<Element, Error> Back(bool pop = false);
    std::pair<Element, Error> Front(bool pop = false);
    std::pair<size_t, Error>  Size();
};

#endif
