#include "session/session.h"

Session::Session()
    : waiting_request_("WAITING_REQUEST", std::bind(&Session::ReceiveRequest, this)),
      request_ready_("REQUEST_READY", std::bind(&Session::ForwardRequest, this)),
      waiting_response_("WAITING_RESPONSE", std::bind(&Session::ReceiveResponse, this)),
      response_ready_("RESPONSE_READY", std::bind(&Session::ForwardResponse, this)),
      error_("ERROR", std::bind(&Session::CloseConnection, this))
{
    SetInitialState(waiting_request_);
}

void Session::operator()()
{
    TakeAction();
}

State* Session::ReceiveRequest()
{
    //   - loop:
    //     - select with 3 seconds timeout
    //       - if error (<0) -> ConnectionLost
    //       - elif timeout (=0) -> ReceiveRequest
    //     - recv
    //       - if error (<0) -> ConnectionLost
    //       - elif done (=0) -> ForwardRequest
    //     - check and resize buffer -> ReceiveRequest
    return &request_ready_;
}

State* Session::ForwardRequest()
{
    return &waiting_response_;
}

State* Session::ReceiveResponse()
{
    return &response_ready_;
}

State* Session::ForwardResponse()
{
    return &error_;
}

State* Session::CloseConnection()
{
    return nullptr;
}
