#include "norves/core/grpc_client.h"
#include "norves/core/async_context.h"

namespace norves::core {

GrpcClient::GrpcClient(ClientConfig config)
    : m_Config(std::move(config))
    , m_Channel(m_Config.Channel)
    , m_Cq(std::make_unique<grpc::CompletionQueue>())
    , m_CqThreads(m_Config.pMemoryResource)
{
}

GrpcClient::~GrpcClient()
{
    Shutdown();
}

Status GrpcClient::Start()
{
    if (m_bRunning.load())
    {
        std::pmr::string errorMsg("Client is already running", m_Config.pMemoryResource);
        return Status{StatusCode::AlreadyExists, std::move(errorMsg)};
    }

    m_bRunning.store(true);

    m_CqThreads.reserve(m_Config.NumCqThreads);
    for (int i = 0; i < m_Config.NumCqThreads; ++i)
    {
        m_CqThreads.emplace_back([this]() { PollCompletionQueue(); });
    }

    return Status::Ok();
}

void GrpcClient::Shutdown()
{
    if (!m_bRunning.exchange(false))
    {
        return;
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

bool GrpcClient::IsRunning() const noexcept
{
    return m_bRunning.load();
}

std::shared_ptr<grpc::Channel> GrpcClient::GetGrpcChannel() const noexcept
{
    return m_Channel.GetChannel();
}

grpc::CompletionQueue* GrpcClient::GetCompletionQueue() noexcept
{
    return m_Cq.get();
}

Status GrpcClient::WaitForConnected(int timeout_ms)
{
    return m_Channel.WaitForConnected(timeout_ms);
}

void GrpcClient::PollCompletionQueue()
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
