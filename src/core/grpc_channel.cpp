#include "norves/core/grpc_channel.h"

#include <chrono>

namespace norves::core {

GrpcChannel::GrpcChannel(ChannelConfig config)
    : m_Config(std::move(config))
{
    auto creds = m_Config.bUseTls
        ? grpc::SslCredentials(grpc::SslCredentialsOptions{})
        : grpc::InsecureChannelCredentials();

    m_Channel = grpc::CreateChannel(std::string(m_Config.Target.c_str()), creds);
}

std::shared_ptr<grpc::Channel> GrpcChannel::GetChannel() const noexcept
{
    return m_Channel;
}

bool GrpcChannel::IsConnected() const
{
    auto state = m_Channel->GetState(false);
    return state == GRPC_CHANNEL_READY;
}

Status GrpcChannel::WaitForConnected(int timeout_ms)
{
    auto deadline = std::chrono::system_clock::now()
                  + std::chrono::milliseconds(timeout_ms);

    m_Channel->GetState(true);

    if (m_Channel->WaitForConnected(deadline))
    {
        return Status::Ok();
    }
    
    std::pmr::string errorMsg("Channel failed to connect to ", m_Config.pMemoryResource);
    errorMsg += m_Config.Target;
    return Status{StatusCode::DeadlineExceeded, std::move(errorMsg)};
}

} // namespace norves::core
