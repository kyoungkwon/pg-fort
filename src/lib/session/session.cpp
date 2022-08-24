#include "session/session.h"

Session::Session(ClientConn* cl_conn, DbConn* db_conn, std::shared_ptr<SchemaTracker> st)
    : cl_conn_(cl_conn),
      db_conn_(db_conn),
      st_(st),
      context_{0},
      pf_(this),
      initiate_("INITIATE", std::bind(&Session::Initiate, this)),
      prep_recv_req_("PREP_RECV_REQ", std::bind(&Session::PrepRecvReq, this)),
      recv_req_("RECV_REQ", std::bind(&Session::RecvReq, this)),
      prep_fwd_req_("PREP_FWD_REQ", std::bind(&Session::PrepFwdReq, this)),
      fwd_req_("FWD_REQ", std::bind(&Session::FwdReq, this)),
      prep_recv_resp_("PREP_RECV_RESP", std::bind(&Session::PrepRecvResp, this)),
      recv_resp_("RECV_RESP", std::bind(&Session::RecvResp, this)),
      prep_fwd_resp_("PREP_FWD_RESP", std::bind(&Session::PrepFwdResp, this)),
      fwd_resp_("FWD_RESP", std::bind(&Session::FwdResp, this))
{
    id = std::rand();
    SetInitialState(prep_recv_req_);
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
    }

    // check last 6 bytes
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

State* Session::Initiate()
{
    std::cout << "[" << id << "] 1:Initiate (from " << cl_conn_->GetSocket() << ")" << std::endl;

    std::cout << "\t\"StartupMessage\"" << std::endl;

    char* data = context_.request_.Data();
    int   size = context_.request_.Size();

    std::cout << "\ttotal size = " << size << std::endl;

    if (size <= 8)
    {
        // empty StartupMessage message, wait for next
        return &prep_fwd_req_;
    }

    int32_t len = (int32_t(data[0]) << 24) + (int32_t(data[1]) << 16) + (int32_t(data[2]) << 8) +
                  int32_t(data[3]);

    std::cout << "\ttotal len = " << len << std::endl;

    int32_t pver = (int32_t(data[4]) << 24) + (int32_t(data[5]) << 16) + (int32_t(data[6]) << 8) +
                   int32_t(data[7]);

    std::cout << "\tprotocol ver = " << pver << std::endl;

    uint32_t pos = 8;
    while (data[pos])
    {
        auto pname = std::string(data + pos);
        pos += pname.length() + 1;
        auto pval = std::string(data + pos);
        pos += pval.length() + 1;

        parameters_[pname] = pval;
        if (pname == "user" && !parameters_.contains("database"))
        {
            parameters_["database"] = pval;
        }
    }

    std::cout << "\tparameters:" << std::endl;
    for (const auto& [k, v] : parameters_)
    {
        std::cout << "\t\t" << k << " = " << v << std::endl;
    }

    context_.initiated_ = true;
    return &prep_fwd_req_;
}

State* Session::PrepRecvReq()
{
    std::cout << "[" << id << "] 1:PrepRecvReq (from " << cl_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    context_.request_.Reset();
    return &recv_req_;
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
    return context_.initiated_ ? &prep_fwd_req_ : &initiate_;
}

// State* Session::ReqPlugin()
// {
//     char* data = context_.request_.Data();
//     int   size = context_.request_.Size();

//     // create a plugin instance with:
//     //  - start_at = &acl_query_
//     //  - return_to = &prep_fwd_req_
// }

State* Session::PrepFwdReq()
{
    std::cout << "[" << id << "] 3:PrepFwdReq (to " << db_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = db_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &fwd_req_;
}

State* Session::FwdReq()
{
    std::cout << "[" << id << "] 4:FwdReq (to " << db_conn_->GetSocket() << ")";

    auto res = db_conn_->ForwardRequest(context_.request_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }
    context_.waiting_ = false;
    return &prep_recv_resp_;
}

State* Session::PrepRecvResp()
{
    std::cout << "[" << id << "] 5:PrepRecvResp (from " << db_conn_->GetSocket() << ")"
              << std::endl;

    context_.ev_.data.fd = db_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    context_.response_.Reset();
    return &recv_resp_;
}

State* Session::RecvResp()
{
    std::cout << "\033[31m"
              << "[" << id << "] 6:RecvResp (from " << db_conn_->GetSocket() << ")\033[0m";

    auto res = db_conn_->ReceiveResponse(context_.response_);

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
        return &recv_resp_;
    }

    context_.waiting_ = false;
    return &prep_fwd_resp_;
}

State* Session::PrepFwdResp()
{
    std::cout << "[" << id << "] 7:PrepFwdResp (to " << cl_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &fwd_resp_;
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
    return &prep_recv_req_;
}
