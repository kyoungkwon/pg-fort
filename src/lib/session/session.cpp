#include "session/session.h"

Session::Session()
    : Session(nullptr, nullptr)
{
}

Session::Session(ClientConn* cl_conn, DbConn* db_conn)
    : cl_conn_(cl_conn),
      db_conn_(db_conn),
      waiting_request_("WAITING_REQUEST", std::bind(&Session::ReceiveRequest, this)),
      request_ready_("REQUEST_READY", std::bind(&Session::ForwardRequest, this)),
      waiting_response_("WAITING_RESPONSE", std::bind(&Session::ReceiveResponse, this)),
      response_ready_("RESPONSE_READY", std::bind(&Session::ForwardResponse, this)),
      done_("DONE", std::bind(&Session::CloseSession, this))
{
    id = std::rand();
    SetInitialState(waiting_request_);
}

void Session::operator()()
{
    TakeAction();
}

State* Session::ReceiveRequest()
{
    std::cout << "[" << id << "] ReceiveRequest: ";

    context_.request_.Reset();
    auto res = cl_conn_->ReceiveRequest(context_.request_);

    std::cout << res << std::endl;

    if (res < 0)
    {
        context_.errno_ = errno;
        return &done_;
    }
    return &request_ready_;
}

State* Session::ForwardRequest()
{
    std::cout << "[" << id << "] ForwardRequest: ";

    auto res = db_conn_->ForwardRequest(context_.request_);

    std::cout << res << std::endl;

    if (res < 0)
    {
        context_.errno_ = errno;
        return &done_;
    }
    return &waiting_response_;
}

State* Session::ReceiveResponse()
{
    std::cout << "[" << id << "] ReceiveResponse: ";

    context_.response_.Reset();
    auto res = db_conn_->ReceiveResponse(context_.response_);

    std::cout << res << std::endl;

    if (res < 0)
    {
        context_.errno_ = errno;
        return &done_;
    }
    return &response_ready_;
}

State* Session::ForwardResponse()
{
    std::cout << "[" << id << "] ForwardResponse: ";

    auto res = cl_conn_->ForwardResponse(context_.response_);

    std::cout << res << std::endl;

    if (res < 0)
    {
        context_.errno_ = errno;
        return &done_;
    }
    return &waiting_request_;
}

State* Session::CloseSession()
{
    std::cout << "[" << id << "] CloseSession" << std::endl;

    // TODO: log error + return error to client
    delete cl_conn_;
    delete db_conn_;
    return nullptr;
}
