#pragma once

#include "norves/core/status.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <string_view>

namespace norves::core {

/// Configuration for creating a gRPC channel.
struct ChannelConfig {
    std::string target;           // e.g. "localhost:50051"
    bool use_tls = false;         // Use TLS credentials (false = insecure)
};

/// RAII wrapper around a gRPC channel, managing connection lifecycle.
class GrpcChannel {
public:
    /// Constructs a channel with the given configuration.
    explicit GrpcChannel(ChannelConfig config);

    ~GrpcChannel() = default;

    // Non-copyable, movable.
    GrpcChannel(const GrpcChannel&) = delete;
    GrpcChannel& operator=(const GrpcChannel&) = delete;
    GrpcChannel(GrpcChannel&&) noexcept = default;
    GrpcChannel& operator=(GrpcChannel&&) noexcept = default;

    /// Returns the underlying gRPC channel (for creating stubs).
    [[nodiscard]] std::shared_ptr<grpc::Channel> GetChannel() const noexcept;

    /// Checks if the channel is currently in a connected/ready state.
    [[nodiscard]] bool IsConnected() const;

    /// Waits for the channel to connect within the given deadline (ms).
    [[nodiscard]] Status WaitForConnected(int timeout_ms = 5000);

    /// Returns the target address.
    [[nodiscard]] const std::string& target() const noexcept { return config_.target; }

private:
    ChannelConfig config_;
    std::shared_ptr<grpc::Channel> channel_;
};

} // namespace norves::core
