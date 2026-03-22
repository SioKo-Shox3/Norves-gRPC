#include "norves/core/grpc_channel.h"

#include <chrono>

namespace norves::core {

GrpcChannel::GrpcChannel(ChannelConfig config)
    : config_(std::move(config))
{
    auto creds = config_.use_tls
        ? grpc::SslCredentials(grpc::SslCredentialsOptions{})
        : grpc::InsecureChannelCredentials();

    channel_ = grpc::CreateChannel(config_.target, creds);
}

std::shared_ptr<grpc::Channel> GrpcChannel::GetChannel() const noexcept {
    return channel_;
}

bool GrpcChannel::IsConnected() const {
    auto state = channel_->GetState(false);
    return state == GRPC_CHANNEL_READY;
}

Status GrpcChannel::WaitForConnected(int timeout_ms) {
    auto deadline = std::chrono::system_clock::now()
                  + std::chrono::milliseconds(timeout_ms);

    channel_->GetState(true);

    if (channel_->WaitForConnected(deadline)) {
        return Status::Ok();
    }
    return Status{StatusCode::DeadlineExceeded,
                  "Channel failed to connect to " + config_.target};
}

} // namespace norves::core
