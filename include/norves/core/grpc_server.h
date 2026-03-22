#pragma once

#include "norves/core/status.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

namespace norves::core {

/// Configuration for creating a gRPC server.
struct ServerConfig {
    std::string listen_address = "0.0.0.0:50051";
    int num_cq_threads = 1;
};

/// RAII wrapper around a gRPC async server.
class GrpcServer {
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

    /// Returns true if the server is currently running.
    [[nodiscard]] bool IsRunning() const noexcept;

    /// Returns the CompletionQueue for async operations.
    [[nodiscard]] grpc::ServerCompletionQueue* GetCompletionQueue() const noexcept;

    /// Returns the resolved port after Start().
    [[nodiscard]] int port() const noexcept { return bound_port_; }

private:
    void PollCompletionQueue();

    ServerConfig config_;
    std::unique_ptr<grpc::ServerBuilder> builder_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    std::vector<std::thread> cq_threads_;
    std::atomic<bool> running_{false};
    int bound_port_ = 0;
};

} // namespace norves::core
