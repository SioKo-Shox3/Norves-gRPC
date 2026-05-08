#pragma once

#include "norves/core/async_context.h"
#include <grpcpp/grpcpp.h>
#include <memory_resource>

namespace norves::protocol {

/// State of an RPC handler.
enum class HandlerState
{
    Create,
    Process,
    Finish
};

/// Base template for server-side unary RPC handlers.
template <typename ServiceT, typename RequestT, typename ResponseT>
class UnaryHandler : public core::IAsyncContext
{
public:
    UnaryHandler(ServiceT* service, grpc::ServerCompletionQueue* cq, std::pmr::memory_resource* memoryResource)
        : m_Service(service)
        , m_Cq(cq)
        , m_State(HandlerState::Create)
        , m_Responder(&m_Context)
        , m_pMemoryResource(memoryResource)
    {
    }

    virtual ~UnaryHandler() = default;

    /// Must be implemented by the derived class to initiate the RPC request (e.g. m_Service->RequestPing(...)).
    virtual void RequestAsync() = 0;

    /// Must be implemented by the derived class to process the request and populate the response.
    virtual void ProcessRequest(const RequestT& request, ResponseT& response) = 0;

    /// Must be implemented to create a new instance of the derived handler class so the server can continue accepting requests.
    virtual void Clone() = 0;

    void OnEvent(bool bOk) override
    {
        if (!bOk)
        {
            delete this;
            return;
        }

        switch (m_State)
        {
            case HandlerState::Create:
            {
                m_State = HandlerState::Process;
                // Clone ourselves to serve the next client.
                Clone();
                
                // Process the current request.
                ProcessRequest(m_Request, m_Response);
                
                // Transition to Finish state.
                m_State = HandlerState::Finish;
                m_Responder.Finish(m_Response, grpc::Status::OK, this);
                break;
            }
            case HandlerState::Process:
            {
                // Unary RPC usually goes straight to Finish.
                break;
            }
            case HandlerState::Finish:
            {
                delete this;
                break;
            }
        }
    }

protected:
    ServiceT* m_Service;
    grpc::ServerCompletionQueue* m_Cq;
    HandlerState m_State;

    grpc::ServerContext m_Context;
    RequestT m_Request;
    ResponseT m_Response;
    grpc::ServerAsyncResponseWriter<ResponseT> m_Responder;
    
    std::pmr::memory_resource* m_pMemoryResource;
};

} // namespace norves::protocol
