#include "session/session.h"

Session::Session(ClientConn* cl_conn, DbConn* db_conn)
    : cl_conn_(cl_conn),
      db_conn_(db_conn),
      waiting_request_("WAITING_REQUEST", std::bind(&Session::ReceiveRequest, this)),
      request_ready_("REQUEST_READY", std::bind(&Session::ForwardRequest, this)),
      waiting_response_("WAITING_RESPONSE", std::bind(&Session::ReceiveResponse, this)),
      response_ready_("RESPONSE_READY", std::bind(&Session::ForwardResponse, this)),
      reset_("RESET", std::bind(&Session::ResetContext, this)),
      error_("ERROR", std::bind(&Session::CloseSession, this))
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
    std::cout << "ReceiveRequest" << std::endl;

    auto res = cl_conn_->ReceiveRequest(context_.request_);
    if (res <= 0)
    {
        context_.errno_ = res < 0 ? errno : ETIMEDOUT;
        return &error_;
    }
    return &request_ready_;
}

State* Session::ForwardRequest()
{
    std::cout << "ForwardRequest" << std::endl;

    auto res = db_conn_->ForwardRequest(context_.request_);
    if (res <= 0)
    {
        context_.errno_ = res < 0 ? errno : ETIMEDOUT;
        return &error_;
    }
    return &waiting_response_;
}

State* Session::ReceiveResponse()
{
    std::cout << "ReceiveResponse" << std::endl;

    auto res = db_conn_->ReceiveResponse(context_.response_);
    if (res <= 0)
    {
        context_.errno_ = res < 0 ? errno : ETIMEDOUT;
        return &error_;
    }
    return &response_ready_;
}

State* Session::ForwardResponse()
{
    std::cout << "ForwardResponse" << std::endl;

    auto res = cl_conn_->ForwardResponse(context_.response_);
    if (res <= 0)
    {
        context_.errno_ = res < 0 ? errno : ETIMEDOUT;
        return &error_;
    }
    return &reset_;
}

State* Session::ResetContext()
{
    std::cout << "ResetContext" << std::endl;
    
    context_.request_.Reset();
    context_.response_.Reset();
    context_.errno_ = 0;
    return &waiting_request_;
}

State* Session::CloseSession()
{
    std::cout << "CloseSession" << std::endl;

    // TODO: log error + return error to client
    delete cl_conn_;
    delete db_conn_;
    return nullptr;
}
