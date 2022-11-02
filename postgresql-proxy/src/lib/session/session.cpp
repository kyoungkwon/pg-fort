#include "session/session.h"

#include "print-info.cpp"

Session::Session(ClientConn* cl_conn, ServerConn* sv_conn, std::shared_ptr<PqxxConnPool> pcp,
                 std::shared_ptr<SchemaTracker> st)
    : cl_conn_(cl_conn),
      sv_conn_(sv_conn),
      pcp_(pcp),
      st_(st),
      context_({0}),
      pf_(this),
      initiate_("INITIATE", std::bind(&Session::Initiate, this)),
      prepare_to_receive_request_("PREPARE_TO_RECEIVE_REQUEST", std::bind(&Session::PrepareToReceiveRequest, this)),
      receive_request_("RECEIVE_REQUEST", std::bind(&Session::ReceiveRequest, this)),
      apply_pre_request_plugins_("APPLY_PRE_REQUEST_PLUGINS", std::bind(&Session::ApplyPreRequestPlugins, this)),
      prepare_to_forward_request_("PREPARE_TO_FORWARD_REQUEST", std::bind(&Session::PrepareToForwardRequest, this)),
      forward_request_("FORWARD_REQUEST", std::bind(&Session::ForwardRequest, this)),
      prepare_to_receive_response_("PREPARE_TO_RECEIVE_RESPONSE", std::bind(&Session::PrepareToReceiveResponse, this)),
      receive_response_("RECEIVE_RESPONSE", std::bind(&Session::ReceiveResponse, this)),
      apply_post_response_plugins_("APPLY_POST_RESPONSE_PLUGINS", std::bind(&Session::ApplyPostResponsePlugins, this)),
      prepare_to_forward_response_("PREPARE_TO_FORWARD_RESPONSE", std::bind(&Session::PrepareToForwardResponse, this)),
      forward_response_("FORWARD_RESPONSE", std::bind(&Session::ForwardResponse, this)),
      reset_context_("RESET_CONTEXT", std::bind(&Session::ResetContext, this))
{
    id = std::rand();
    SetInitialState(prepare_to_receive_request_);

    // clang-format off
    pre_request_plugins_ = {
        pf_.TranslateProxyCommandPlugIn(),
        pf_.ParseQueryPlugIn(),
        pf_.AclQueryPlugIn(),
        pf_.DropTablePlugIn(),
        pf_.EnsureNewTableHasIdPlugIn(),
        pf_.RestrictInternalTableAccessPlugIn()
    };

    post_response_plugins_ = {
        pf_.UpdateParametersPlugIn(),
        pf_.UpdateSchemaPlugIn()
    };
    // clang-format on
}

Session::~Session()
{
    std::cout << "Deleting session [" << id << "]" << std::endl;
    delete cl_conn_;
    delete sv_conn_;
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

State* Session::Initiate()
{
    std::cout << "[" << id << "] 0:Initiate (from " << cl_conn_->GetSocket() << ")" << std::endl;

    std::cout << "\t\"StartupMessage\"" << std::endl;

    char* data = context_.request_.Data();
    int   size = context_.request_.Size();

    std::cout << "\ttotal size = " << size << std::endl;

    if (size <= 8)
    {
        // empty StartupMessage message, wait for next
        return &prepare_to_forward_request_;
    }

    int32_t len = (int32_t(data[0]) << 24) + (int32_t(data[1]) << 16) + (int32_t(data[2]) << 8) + int32_t(data[3]);
    std::cout << "\ttotal len = " << len << std::endl;

    int32_t pver = (int32_t(data[4]) << 24) + (int32_t(data[5]) << 16) + (int32_t(data[6]) << 8) + int32_t(data[7]);
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

    // is user a superuser?
    pqxx::row r;
    {
        auto       pqxx = (pcp_->Acquire());
        pqxx::work w(*pqxx);
        r = w.exec1("SELECT rolsuper FROM pg_roles WHERE rolname = '" + parameters_["user"] + "'");
        w.commit();
    }
    parameters_["rolsuper"] = r["rolsuper"].c_str();

    std::cout << "INITIALIZEINITIALIZEINITIALIZEINITIALIZEINITIALIZEINITIALIZEINITIALIZE\n";
    std::cout << "\tparameters:" << std::endl;
    for (const auto& [k, v] : parameters_)
    {
        std::cout << "\t\t" << k << " = " << v << std::endl;
    }

    context_.initiated_ = true;
    return &prepare_to_forward_request_;
}

State* Session::PrepareToReceiveRequest()
{
    std::cout << "[" << id << "] 1:PrepareToReceiveRequest (from " << cl_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    return &receive_request_;
}

State* Session::ReceiveRequest()
{
    std::cout << "[" << id << "] 2:ReceiveRequest (from " << cl_conn_->GetSocket() << ")";

    auto res = cl_conn_->ReceiveRequest(context_.request_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }

    PrintInfo(FRONTEND, context_.request_.Data(), res);

    context_.waiting_ = false;
    return context_.initiated_ ? &apply_pre_request_plugins_ : &initiate_;
}

State* Session::ApplyPreRequestPlugins()
{
    std::cout << "[" << id << "] 3:ApplyPreRequestPlugins" << std::endl;

    try
    {
        for (auto& p : pre_request_plugins_)
        {
            auto err = p.Apply();
            if (err)
            {
                context_.response_.SetError(err);
                PrintInfo(BACKEND, context_.response_.Data(), context_.response_.Size());
                return &prepare_to_forward_response_;
            }
        }
    }
    catch (std::exception& e)
    {
        // unexpected internal error
        context_.response_.SetError(Error({
            {"S",  "ERROR"},
            {"C",  "XX000"},
            {"M", e.what()}
        }));
        PrintInfo(BACKEND, context_.response_.Data(), context_.response_.Size());
        return &prepare_to_forward_response_;
    }

    return &prepare_to_forward_request_;
}

State* Session::PrepareToForwardRequest()
{
    std::cout << "[" << id << "] 4:PrepareToForwardRequest (to " << sv_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = sv_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &forward_request_;
}

State* Session::ForwardRequest()
{
    std::cout << "[" << id << "] 5:ForwardRequest (to " << sv_conn_->GetSocket() << ")";

    auto res = sv_conn_->ForwardRequest(context_.request_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }
    context_.waiting_ = false;
    return &prepare_to_receive_response_;
}

State* Session::PrepareToReceiveResponse()
{
    std::cout << "[" << id << "] 6:PrepareToReceiveResponse (from " << sv_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = sv_conn_->GetSocket();
    context_.ev_.events  = EPOLLIN;
    context_.waiting_    = true;
    return &receive_response_;
}

State* Session::ReceiveResponse()
{
    std::cout << "\033[31m"
              << "[" << id << "] 7:ReceiveResponse (from " << sv_conn_->GetSocket() << ")\033[0m";

    auto res = sv_conn_->ReceiveResponse(context_.response_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }

    PrintInfo(BACKEND, context_.response_.Data(), res);

    // HACK: pgcli will hang unless received ReadyForQuery message from db
    auto ready_for_query = [&](char* data, int size)
    {
        return !context_.initiated_ || data[0] == 'R' ||
               (data[size - 6] == 'Z' && data[size - 5] == 0 && data[size - 4] == 0 && data[size - 3] == 0 &&
                data[size - 2] == 5 && (data[size - 1] == 'I' || data[size - 1] == 'T' || data[size - 1] == 'E'));
    };

    if (!ready_for_query(context_.response_.Data(), res))
    {
        return &receive_response_;
    }

    context_.waiting_ = false;
    return &apply_post_response_plugins_;
}

State* Session::ApplyPostResponsePlugins()
{
    std::cout << "[" << id << "] 8:ApplyPostResponsePlugins" << std::endl;

    try
    {
        for (auto& p : post_response_plugins_)
        {
            p.Apply();
        }
    }
    catch (std::exception& e)
    {
        // unexpected internal error
        context_.response_.SetError(Error({
            {"S",  "ERROR"},
            {"C",  "XX000"},
            {"M", e.what()}
        }));
        PrintInfo(BACKEND, context_.response_.Data(), context_.response_.Size());
    }
    return &prepare_to_forward_response_;
}

State* Session::PrepareToForwardResponse()
{
    std::cout << "[" << id << "] 9:PrepareToForwardResponse (to " << cl_conn_->GetSocket() << ")" << std::endl;

    context_.ev_.data.fd = cl_conn_->GetSocket();
    context_.ev_.events  = EPOLLOUT;
    context_.waiting_    = true;
    return &forward_response_;
}

State* Session::ForwardResponse()
{
    std::cout << "[" << id << "] 10:ForwardResponse (to " << cl_conn_->GetSocket() << ")";

    auto res = cl_conn_->ForwardResponse(context_.response_);

    std::cout << "\t" << res << std::endl;

    if (res <= 0)
    {
        context_.errno_ = errno;
        return nullptr;
    }
    context_.waiting_ = false;
    return &reset_context_;
}

State* Session::ResetContext()
{
    std::cout << "[" << id << "] 11:ResetContext" << std::endl;

    context_.Reset();
    return &prepare_to_receive_request_;
}
