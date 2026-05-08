#include "norves/core/grpc_server.h"
#include "norves/core/async_context.h"

#include <iostream>

namespace norves::core {

GrpcServer::GrpcServer(ServerConfig config)
    : m_Config(std::move(config))
    , m_Builder(std::make_unique<grpc::ServerBuilder>())
    , m_CqThreads(m_Config.pMemoryResource)
{
}

GrpcServer::~GrpcServer()
{
    Shutdown();
}

void GrpcServer::RegisterService(grpc::Service* service)
{
    m_Builder->RegisterService(service);
}

Status GrpcServer::Start()
{
    if (m_bRunning.load())
    {
        std::pmr::string errorMsg("Server is already running", m_Config.pMemoryResource);
        return Status{StatusCode::AlreadyExists, std::move(errorMsg)};
    }

    m_Builder->AddListeningPort(std::string(m_Config.ListenAddress.c_str()),
                               grpc::InsecureServerCredentials(),
                               &m_BoundPort);

    m_Cq = m_Builder->AddCompletionQueue();

    m_Server = m_Builder->BuildAndStart();
    if (!m_Server)
    {
        std::pmr::string errorMsg("Failed to start server on ", m_Config.pMemoryResource);
        errorMsg += m_Config.ListenAddress;
        return Status{StatusCode::Internal, std::move(errorMsg)};
    }

    m_bRunning.store(true);

    m_CqThreads.reserve(m_Config.NumCqThreads);
    for (int i = 0; i < m_Config.NumCqThreads; ++i)
    {
        m_CqThreads.emplace_back([this]() { PollCompletionQueue(); });
    }

    return Status::Ok();
}

void GrpcServer::Shutdown()
{
    if (!m_bRunning.exchange(false))
    {
        return;
    }

    if (m_Server)
    {
        m_Server->Shutdown();
    }
    if (m_Cq)
    {
        m_Cq->Shutdown();
    }

    for (auto& t : m_CqThreads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    m_CqThreads.clear();

    if (m_Cq)
    {
        void* tag = nullptr;
        bool ok = false;
        while (m_Cq->Next(&tag, &ok))
        {
        }
    }
}

bool GrpcServer::IsRunning() const noexcept
{
    return m_bRunning.load();
}

grpc::ServerCompletionQueue* GrpcServer::GetCompletionQueue() const noexcept
{
    return m_Cq.get();
}

void GrpcServer::PollCompletionQueue()
{
    void* tag = nullptr;
    bool ok = false;

    while (m_bRunning.load())
    {
        auto deadline = std::chrono::system_clock::now()
                      + std::chrono::milliseconds(100);
        auto result = m_Cq->AsyncNext(&tag, &ok, deadline);

        if (result == grpc::CompletionQueue::GOT_EVENT)
        {
            if (tag)
            {
                auto* handler = static_cast<IAsyncContext*>(tag);
                handler->OnEvent(ok);
            }
        }
        else if (result == grpc::CompletionQueue::SHUTDOWN)
        {
            break;
        }
    }
}

} // namespace norves::core
