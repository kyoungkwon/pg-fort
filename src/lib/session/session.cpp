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

enum SentBy
{
    BACKEND,
    FRONTEND
};

void PrintInfo(SentBy src, char* data, int size)
{
    auto type = data[0];
    std::cout << "type = " << type << std::endl;

    if (type == 'Q')
    {
        std::cout << "\t\"Query\"" << std::endl;

        int32_t len = (uint32_t(data[1]) << 24) + (uint32_t(data[2]) << 16) +
                      (uint32_t(data[3]) << 8) + uint32_t(data[4]);

        std::cout << "\ttotal len = " << len << std::endl;

        len -= 4;

        std::cout << "\tquery len = " << len << std::endl;

        auto query = data + 5;

        std::cout << "\n\tquery =\n\t" << query << std::endl;
    }
    else if (type == 'E' && src == BACKEND)
    {
        std::cout << "\t\033[1;35m\"ErrorResponse\"\033[0m" << std::endl;

        uint32_t len = (uint32_t(data[1]) << 24) + (uint32_t(data[2]) << 16) +
                       (uint32_t(data[3]) << 8) + uint32_t(data[4]);

        std::cout << "\ttotal len = " << len << std::endl;
        std::cout << "\tbody len = " << len - 4 << std::endl;

        uint32_t pos = 5;
        while (pos < len)
        {
            std::cout << "\tfield type = " << data[pos] << std::endl;

            pos += 1;
            char* v = data + pos;

            std::cout << "\tfield value = " << v;
            std::cout << "\t[pos: " << pos << "]" << std::endl;

            pos += strlen(v) + 1;
        }
        std::cout << "\tdone [pos: " << pos << "]" << std::endl;

        for (int i = pos; i < size; i++)
        {
            std::cout << "\t\tdata[" << i << "] = " << data[i];
            printf(" [%x]", data[i]);
            std::cout << (data[i] == '\0' ? " (zero)" : "") << std::endl;
        }
    }
    else if (type == 'T' && src == BACKEND)
    {
        std::cout << "\t\033[1;33m\"RowDescription\"\033[0m" << std::endl;

        uint32_t len = (uint32_t(data[1]) << 24) + (uint32_t(data[2]) << 16) +
                       (uint32_t(data[3]) << 8) + uint32_t(data[4]);

        std::cout << "\ttotal len = " << len << std::endl;

        uint32_t nfield = (uint32_t(data[5]) << 8) + uint32_t(data[6]);

        std::cout << "\tnum fields = " << nfield << std::endl;

        uint32_t pos = 7;
        for (int32_t i = 0; i < nfield; i++)
        {
            char* name = data + pos;

            std::cout << "\t\tfields[" << i << "].name = " << name;
            std::cout << "\t[pos: " << pos << "]" << std::endl;

            pos += strlen(name) + 1 + 4 + 2 + 4 + 2 + 4 + 2;
        }
        std::cout << "\t\tdone [pos: " << pos << "]" << std::endl;

        // TODO: what comes after pos?
    }

    if (size > 6)
    {
        uint32_t i = size - 6;
        std::cout << "\tlast 6 bytes:" << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
    }

    std::cout << std::endl;
}

State* Session::PrepRecvReq()
{
    std::cout << "[" << id << "] 1:PrepRecvReq (from " << cl_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    context_.request_.Reset();
    return &recv_req;
}

State* Session::RecvReq()
{
    std::cout << "[" << id << "] 2:RecvReq (from " << cl_conn_->GetSocket() << ")";

    auto res = cl_conn_->ReceiveRequest(context_.request_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }

    PrintInfo(FRONTEND, context_.request_.Data(), res);

    context_.waiting_ = false;
    return &prep_fwd_req;
}

State* Session::PrepFwdReq()
{
    std::cout << "[" << id << "] 3:PrepFwdReq (to " << db_conn_->GetSocket() << ")" << std::endl;

    // TODO: db_conn_ = db_conn_pool_.acquire()
    context_.ev_.data.fd = db_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &fwd_req;
}

State* Session::FwdReq()
{
    std::cout << "[" << id << "] 4:FwdReq (to " << db_conn_->GetSocket() << ")";

    auto res = db_conn_->ForwardRequest(context_.request_);
    // TODO: db_conn_.release()

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
    std::cout << "[" << id << "] 5:PrepRecvResp (from " << db_conn_->GetSocket() << ")"
              << std::endl;

    // TODO: db_conn_ = db_conn_pool_.acquire()
    context_.ev_.data.fd = db_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    context_.response_.Reset();
    return &recv_resp;
}

State* Session::RecvResp()
{
    std::cout << "\033[31m"
              << "[" << id << "] 6:RecvResp (from " << db_conn_->GetSocket() << ")\033[0m";

    auto res = db_conn_->ReceiveResponse(context_.response_);
    // TODO: db_conn_.release()

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }

    PrintInfo(BACKEND, context_.response_.Data(), res);

    // HACK: pgcli will hang unless received ReadyForQuery message from db
    auto ready_for_query = [](char* data, int size)
    {
        return data[0] != 'E' ||  // only happens with ErrorResponse
               (data[size - 6] == 'Z' && data[size - 5] == 0 && data[size - 4] == 0 &&
                data[size - 3] == 0 && data[size - 2] == 5 &&
                (data[size - 1] == 'I' || data[size - 1] == 'T' || data[size - 1] == 'E'));
    };

    if (!ready_for_query(context_.response_.Data(), res))
    {
        return &recv_resp;
    }

    context_.waiting_ = false;
    return &prep_fwd_resp;
}

State* Session::PrepFwdResp()
{
    std::cout << "[" << id << "] 7:PrepFwdResp (to " << cl_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &fwd_resp;
}

State* Session::FwdResp()
{
    std::cout << "[" << id << "] 8:FwdResp (to " << cl_conn_->GetSocket() << ")";

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
