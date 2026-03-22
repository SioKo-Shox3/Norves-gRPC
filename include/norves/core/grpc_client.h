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

namespace norves::core {

/// Configuration for a gRPC client.
struct ClientConfig {
    ChannelConfig channel;
    int completion_queue_threads = 1;
};

/// RAII wrapper around a gRPC async client.
class GrpcClient {
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
    [[nodiscard]] GrpcChannel& GetChannel() noexcept { return channel_; }
    [[nodiscard]] const GrpcChannel& GetChannel() const noexcept { return channel_; }

    /// Returns the shared gRPC channel (for creating stubs).
    [[nodiscard]] std::shared_ptr<grpc::Channel> GetGrpcChannel() const noexcept;

    /// Returns the CompletionQueue for async RPC calls.
    [[nodiscard]] grpc::CompletionQueue* GetCompletionQueue() noexcept;

    /// Waits for the channel to be connected.
    [[nodiscard]] Status WaitForConnected(int timeout_ms = 5000);

private:
    void PollCompletionQueue();

    ClientConfig config_;
    GrpcChannel channel_;
    std::unique_ptr<grpc::CompletionQueue> cq_;
    std::vector<std::thread> cq_threads_;
    std::atomic<bool> running_{false};
};

} // namespace norves::core
