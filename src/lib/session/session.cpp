#include "session/session.h"

Session::Session(ClientConn* cl_conn, DbConn* db_conn)
    : cl_conn_(cl_conn),
      db_conn_(db_conn),
      prep_recv_req("PREP_RECV_REQ", std::bind(&Session::PrepRecvReq, this)),
      recv_req("RECV_REQ", std::bind(&Session::RecvReq, this)),
      prep_fwd_req("PREP_FWD_REQ", std::bind(&Session::PrepFwdReq, this)),
      fwd_req("FWD_REQ", std::bind(&Session::FwdReq, this)),
      prep_recv_resp("PREP_RECV_RESP", std::bind(&Session::PrepRecvResp, this)),
      recv_resp("RECV_RESP", std::bind(&Session::RecvResp, this)),
      prep_fwd_resp("PREP_FWD_RESP", std::bind(&Session::PrepFwdResp, this)),
      fwd_resp("FWD_RESP", std::bind(&Session::FwdResp, this))
{
    id = std::rand();
    SetInitialState(prep_recv_req);
}

Session::~Session()
{
    delete cl_conn_;
    delete db_conn_;
}

void Session::operator()()
{
    TakeAction();
}

bool Session::IsWaiting()
{
    return context_.waiting_;
}

struct pollfd Session::GetPollFd()
{
    return context_.pollfd_;
}

State* Session::PrepRecvReq()
{
    context_.pollfd_.fd     = cl_conn_->GetSocket();
    context_.pollfd_.events = POLLIN;
    context_.waiting_       = true;
    context_.request_.Reset();
    return &recv_req;
}

State* Session::RecvReq()
{
    auto res = cl_conn_->ReceiveRequest(context_.request_);
    if (res <= 0)
    {
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_fwd_req;
}

State* Session::PrepFwdReq()
{
    context_.pollfd_.fd     = db_conn_->GetSocket();
    context_.pollfd_.events = POLLOUT;
    context_.waiting_       = true;
    return &fwd_req;
}

State* Session::FwdReq()
{
    auto res = db_conn_->ForwardRequest(context_.request_);
    if (res <= 0)
    {
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_recv_resp;
}

State* Session::PrepRecvResp()
{
    context_.pollfd_.fd     = db_conn_->GetSocket();
    context_.pollfd_.events = POLLIN;
    context_.waiting_       = true;
    context_.response_.Reset();
    return &recv_resp;
}

State* Session::RecvResp()
{
    auto res = db_conn_->ReceiveResponse(context_.response_);
    if (res <= 0)
    {
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_fwd_resp;
}

State* Session::PrepFwdResp()
{
    context_.pollfd_.fd     = cl_conn_->GetSocket();
    context_.pollfd_.events = POLLOUT;
    context_.waiting_       = true;
    return &fwd_resp;
}

State* Session::FwdResp()
{
    auto res = cl_conn_->ForwardResponse(context_.response_);
    if (res <= 0)
    {
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_recv_req;
}
