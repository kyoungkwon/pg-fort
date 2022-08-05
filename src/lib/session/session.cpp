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
    std::cout << "Deleting session [" << id << "]" << std::endl;
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

struct epoll_event Session::GetEpollEvent()
{
    return context_.ev_;
}

State* Session::PrepRecvReq()
{
    std::cout << "[" << id << "] 1:PrepRecvReq" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    context_.request_.Reset();
    return &recv_req;
}

State* Session::RecvReq()
{
    std::cout << "[" << id << "] 2:RecvReq";

    auto res = cl_conn_->ReceiveRequest(context_.request_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }

    auto data = context_.request_.Data();
    auto type = data[0];
    std::cout << "type = " << type << std::endl;

    if (type == 'Q')
    {
        std::cout << "\t\"Query\"" << std::endl;

        int32_t len = (uint32_t(data[1]) << 24) + (uint32_t(data[2]) << 16) +
                      (uint32_t(data[3]) << 8) + uint32_t(data[4]) - 4;

        std::cout << "\tlen = " << len << std::endl;

        auto query = (unsigned char*)malloc(len + 1);
        memcpy(query, data + 5, len);
        query[len] = '\0';

        std::cout << "\tquery = " << query << std::endl;

        free(query);
    }

    std::cout << std::endl;

    context_.waiting_ = false;
    return &prep_fwd_req;
}

State* Session::PrepFwdReq()
{
    std::cout << "[" << id << "] 3:PrepFwdReq" << std::endl;

    context_.ev_.data.fd = db_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &fwd_req;
}

State* Session::FwdReq()
{
    std::cout << "[" << id << "] 4:FwdReq";

    auto res = db_conn_->ForwardRequest(context_.request_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_recv_resp;
}

State* Session::PrepRecvResp()
{
    std::cout << "[" << id << "] 5:PrepRecvResp" << std::endl;

    context_.ev_.data.fd = db_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    context_.response_.Reset();
    return &recv_resp;
}

State* Session::RecvResp()
{
    std::cout << "[" << id << "] 6:RecvResp";

    auto res = db_conn_->ReceiveResponse(context_.response_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_fwd_resp;
}

State* Session::PrepFwdResp()
{
    std::cout << "[" << id << "] 7:PrepFwdResp" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &fwd_resp;
}

State* Session::FwdResp()
{
    std::cout << "[" << id << "] 8:FwdResp";

    auto res = cl_conn_->ForwardResponse(context_.response_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_recv_req;
}
