#pragma once

#include "norves/core/async_context.h"
#include <grpcpp/grpcpp.h>
#include <memory_resource>

namespace norves::protocol {

enum class BidiStreamState
{
    Create,
    Connected,
    Read,
    Write,
    Finish,
    Done
};

/// Base template for server-side bidirectional stream RPC handlers.
template <typename ServiceT, typename RequestT, typename ResponseT>
class BidiStreamHandler : public core::IAsyncContext
{
public:
    BidiStreamHandler(ServiceT* service, grpc::ServerCompletionQueue* cq, std::pmr::memory_resource* memoryResource)
        : m_Service(service)
        , m_Cq(cq)
        , m_State(BidiStreamState::Create)
        , m_ReaderWriter(&m_Context)
        , m_pMemoryResource(memoryResource)
    {
    }

    virtual ~BidiStreamHandler() = default;

    virtual void RequestAsync() = 0;
    virtual void Clone() = 0;

    virtual void OnConnected() = 0;
    virtual void OnMessageReceived(const RequestT& request) = 0;
    virtual void OnDisconnected() = 0;

    void Write(const ResponseT& response)
    {
        // Simple implementation: normally needs a queue to avoid overlapping writes
        m_ReaderWriter.Write(response, this);
        // Note: Full robust implementation requires a write queue and separate Write state.
    }

    void OnEvent(bool bOk) override
    {
        if (!bOk)
        {
            if (m_State != BidiStreamState::Create)
            {
                OnDisconnected();
            }
            delete this;
            return;
        }

        switch (m_State)
        {
            case BidiStreamState::Create:
            {
                m_State = BidiStreamState::Connected;
                Clone();
                OnConnected();
                
                // Start reading immediately
                m_State = BidiStreamState::Read;
                m_ReaderWriter.Read(&m_Request, this);
                break;
            }
            case BidiStreamState::Read:
            {
                OnMessageReceived(m_Request);
                // Continue reading
                m_ReaderWriter.Read(&m_Request, this);
                break;
            }
            case BidiStreamState::Write:
            {
                // Write completed, usually return to Read or just idle
                break;
            }
            case BidiStreamState::Finish:
            {
                m_ReaderWriter.Finish(grpc::Status::OK, this);
                m_State = BidiStreamState::Done;
                break;
            }
            case BidiStreamState::Done:
            {
                OnDisconnected();
                delete this;
                break;
            }
            default:
                break;
        }
    }

protected:
    ServiceT* m_Service;
    grpc::ServerCompletionQueue* m_Cq;
    BidiStreamState m_State;

    grpc::ServerContext m_Context;
    RequestT m_Request;
    grpc::ServerAsyncReaderWriter<ResponseT, RequestT> m_ReaderWriter;
    
    std::pmr::memory_resource* m_pMemoryResource;
};

} // namespace norves::protocol
