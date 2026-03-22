/// @file echo_server/main.cpp
/// Minimal echo server demonstrating Norves-gRPC Core layer usage.

#include "norves/core/grpc_server.h"
#include "core.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>

static std::atomic<bool> g_shutdown_requested{false};

void SignalHandler(int /*signal*/) {
    g_shutdown_requested.store(true);
}

/// Synchronous CoreService implementation for the echo example.
class EchoCoreServiceImpl final : public norves::core::CoreService::Service {
public:
    grpc::Status Ping(grpc::ServerContext* /*context*/,
                      const norves::core::PingRequest* request,
                      norves::core::PingResponse* response) override {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        response->set_client_timestamp(request->timestamp());
        response->set_server_timestamp(now);

        std::cout << "[Server] Ping received (client_ts="
                  << request->timestamp() << "), replied with server_ts="
                  << now << std::endl;
        return grpc::Status::OK;
    }

    grpc::Status Echo(grpc::ServerContext* /*context*/,
                      grpc::ServerReaderWriter<norves::core::EchoMessage,
                                              norves::core::EchoMessage>* stream) override {
        norves::core::EchoMessage msg;
        while (stream->Read(&msg)) {
            std::cout << "[Server] Echo: \"" << msg.content() << "\"" << std::endl;

            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            msg.set_timestamp(now);
            stream->Write(msg);
        }
        return grpc::Status::OK;
    }
};

int main(int argc, char** argv) {
    std::string address = "127.0.0.1:50051";
    if (argc > 1) {
        address = argv[1];
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    EchoCoreServiceImpl service;

    norves::core::ServerConfig cfg;
    cfg.listen_address = address;
    cfg.num_cq_threads = 1;

    norves::core::GrpcServer server(cfg);
    server.RegisterService(&service);

    auto status = server.Start();
    if (!status.ok()) {
        std::cerr << "Failed to start server: " << status << std::endl;
        return 1;
    }

    std::cout << "=== Norves Echo Server ===" << std::endl;
    std::cout << "Listening on " << address
              << " (port=" << server.port() << ")" << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    while (!g_shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "\nShutting down..." << std::endl;
    server.Shutdown();
    std::cout << "Server stopped." << std::endl;

    return 0;
}
