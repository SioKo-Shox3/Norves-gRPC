#include "norves/core/grpc_server.h"

#include <iostream>

namespace norves::core {

GrpcServer::GrpcServer(ServerConfig config)
    : config_(std::move(config))
    , builder_(std::make_unique<grpc::ServerBuilder>())
{
}

GrpcServer::~GrpcServer() {
    Shutdown();
}

void GrpcServer::RegisterService(grpc::Service* service) {
    builder_->RegisterService(service);
}

Status GrpcServer::Start() {
    if (running_.load()) {
        return Status{StatusCode::AlreadyExists, "Server is already running"};
    }

    builder_->AddListeningPort(config_.listen_address,
                               grpc::InsecureServerCredentials(),
                               &bound_port_);

    cq_ = builder_->AddCompletionQueue();

    server_ = builder_->BuildAndStart();
    if (!server_) {
        return Status{StatusCode::Internal,
                      "Failed to start server on " + config_.listen_address};
    }

    running_.store(true);

    for (int i = 0; i < config_.num_cq_threads; ++i) {
        cq_threads_.emplace_back([this]() { PollCompletionQueue(); });
    }

    return Status::Ok();
}

void GrpcServer::Shutdown() {
    if (!running_.exchange(false)) {
        return;
    }

    if (server_) {
        server_->Shutdown();
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

bool GrpcServer::IsRunning() const noexcept {
    return running_.load();
}

grpc::ServerCompletionQueue* GrpcServer::GetCompletionQueue() const noexcept {
    return cq_.get();
}

void GrpcServer::PollCompletionQueue() {
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
