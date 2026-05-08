#pragma once

#include "norves/core/status.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <string_view>
#include <memory_resource>

namespace norves::core {

/// Configuration for creating a gRPC channel.
struct ChannelConfig
{
    std::pmr::string Target;      // e.g. "localhost:50051"
    bool bUseTls = false;         // Use TLS credentials (false = insecure)
    std::pmr::memory_resource* pMemoryResource = std::pmr::get_default_resource();
};

/// RAII wrapper around a gRPC channel, managing connection lifecycle.
class GrpcChannel
{
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
    [[nodiscard]] const std::pmr::string& GetTarget() const noexcept
    {
        return m_Config.Target;
    }

private:
    ChannelConfig m_Config;
    std::shared_ptr<grpc::Channel> m_Channel;
};

} // namespace norves::core
