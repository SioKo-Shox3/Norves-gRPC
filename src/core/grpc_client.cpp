#include "norves/core/grpc_client.h"

namespace norves::core {

GrpcClient::GrpcClient(ClientConfig config)
    : config_(std::move(config))
    , channel_(config_.channel)
    , cq_(std::make_unique<grpc::CompletionQueue>())
{
}

GrpcClient::~GrpcClient() {
    Shutdown();
}

Status GrpcClient::Start() {
    if (running_.load()) {
        return Status{StatusCode::AlreadyExists, "Client is already running"};
    }

    running_.store(true);

    for (int i = 0; i < config_.completion_queue_threads; ++i) {
        cq_threads_.emplace_back([this]() { PollCompletionQueue(); });
    }

    return Status::Ok();
}

void GrpcClient::Shutdown() {
    if (!running_.exchange(false)) {
        return;
    }

    if (cq_) {
        cq_->Shutdown();
    }

    for (auto& t : cq_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    cq_threads_.clear();

    if (cq_) {
        void* tag = nullptr;
        bool ok = false;
        while (cq_->Next(&tag, &ok)) {}
    }
}

bool GrpcClient::IsRunning() const noexcept {
    return running_.load();
}

std::shared_ptr<grpc::Channel> GrpcClient::GetGrpcChannel() const noexcept {
    return channel_.GetChannel();
}

grpc::CompletionQueue* GrpcClient::GetCompletionQueue() noexcept {
    return cq_.get();
}

Status GrpcClient::WaitForConnected(int timeout_ms) {
    return channel_.WaitForConnected(timeout_ms);
}

void GrpcClient::PollCompletionQueue() {
    void* tag = nullptr;
    bool ok = false;

    while (running_.load()) {
        auto deadline = std::chrono::system_clock::now()
                      + std::chrono::milliseconds(100);
        auto result = cq_->AsyncNext(&tag, &ok, deadline);

        if (result == grpc::CompletionQueue::GOT_EVENT) {
            if (tag) {
                auto* handler = static_cast<std::function<void(bool)>*>(tag);
                (*handler)(ok);
            }
        } else if (result == grpc::CompletionQueue::SHUTDOWN) {
            break;
        }
    }
}

} // namespace norves::core
