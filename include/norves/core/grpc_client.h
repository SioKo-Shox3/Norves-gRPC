#pragma once

#include "norves/core/status.h"
#include "norves/core/grpc_channel.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <memory_resource>

namespace norves::core {

/// Configuration for a gRPC client.
struct ClientConfig
{
    ChannelConfig Channel;
    int NumCqThreads = 1;
    std::pmr::memory_resource* pMemoryResource = std::pmr::get_default_resource();
};

/// RAII wrapper around a gRPC async client.
class GrpcClient
{
public:
    explicit GrpcClient(ClientConfig config);
    ~GrpcClient();

    GrpcClient(const GrpcClient&) = delete;
    GrpcClient& operator=(const GrpcClient&) = delete;
    GrpcClient(GrpcClient&&) = delete;
    GrpcClient& operator=(GrpcClient&&) = delete;

    /// Starts the client (begins CQ polling).
    [[nodiscard]] Status Start();

    /// Shuts down the client.
    void Shutdown();

    /// Returns true if the client is running.
    [[nodiscard]] bool IsRunning() const noexcept;

    /// Returns the underlying channel wrapper.
    [[nodiscard]] GrpcChannel& GetChannel() noexcept
    {
        return m_Channel;
    }
    
    [[nodiscard]] const GrpcChannel& GetChannel() const noexcept
    {
        return m_Channel;
    }

    /// Returns the shared gRPC channel (for creating stubs).
    [[nodiscard]] std::shared_ptr<grpc::Channel> GetGrpcChannel() const noexcept;

    /// Returns the CompletionQueue for async RPC calls.
    [[nodiscard]] grpc::CompletionQueue* GetCompletionQueue() noexcept;

    /// Waits for the channel to be connected.
    [[nodiscard]] Status WaitForConnected(int timeout_ms = 5000);

private:
    void PollCompletionQueue();

    ClientConfig m_Config;
    GrpcChannel m_Channel;
    std::unique_ptr<grpc::CompletionQueue> m_Cq;
    std::pmr::vector<std::thread> m_CqThreads;
    std::atomic<bool> m_bRunning{false};
};

} // namespace norves::core
