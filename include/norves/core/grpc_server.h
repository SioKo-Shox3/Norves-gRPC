#pragma once

#include "norves/core/status.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <memory_resource>

#include "norves/core/double_buffered_queue.h"

namespace norves::core {

/// Configuration for creating a gRPC server.
struct ServerConfig
{
    std::pmr::string ListenAddress = "0.0.0.0:50051";
    int NumCqThreads = 1;
    std::pmr::memory_resource* pMemoryResource = std::pmr::get_default_resource();
};

/// RAII wrapper around a gRPC async server.
class GrpcServer
{
public:
    explicit GrpcServer(ServerConfig config);
    ~GrpcServer();

    GrpcServer(const GrpcServer&) = delete;
    GrpcServer& operator=(const GrpcServer&) = delete;
    GrpcServer(GrpcServer&&) = delete;
    GrpcServer& operator=(GrpcServer&&) = delete;

    /// Registers a gRPC service. Must be called before Start().
    void RegisterService(grpc::Service* service);

    /// Starts the server and begins processing RPCs.
    [[nodiscard]] Status Start();

    /// Gracefully shuts down the server.
    void Shutdown();

    /// Processes queued tasks on the calling thread (e.g., game main loop).
    void Tick();

    /// Posts a task to be executed on the main thread during the next Tick().
    void PostToMainThread(std::function<void()> task);

    /// Returns true if the server is currently running.
    [[nodiscard]] bool IsRunning() const noexcept;

    /// Returns the CompletionQueue for async operations.
    [[nodiscard]] grpc::ServerCompletionQueue* GetCompletionQueue() const noexcept;

    /// Returns the resolved port after Start().
    [[nodiscard]] int GetPort() const noexcept
    {
        return m_BoundPort;
    }

private:
    void PollCompletionQueue();

    ServerConfig m_Config;
    std::unique_ptr<grpc::ServerBuilder> m_Builder;
    std::unique_ptr<grpc::Server> m_Server;
    std::unique_ptr<grpc::ServerCompletionQueue> m_Cq;
    std::pmr::vector<std::thread> m_CqThreads;
    std::atomic<bool> m_bRunning{false};
    int m_BoundPort = 0;
    DoubleBufferedQueue<std::function<void()>> m_MainThreadTasks;
};

} // namespace norves::core
