/// @file echo_client/main.cpp
/// Minimal echo client demonstrating Norves-gRPC Core layer usage.

#include "norves/core/grpc_client.h"
#include "core.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    std::string target = "127.0.0.1:50051";
    if (argc > 1) {
        target = argv[1];
    }

    norves::core::ClientConfig cfg;
    cfg.channel.target = target;
    cfg.channel.use_tls = false;
    cfg.completion_queue_threads = 1;

    norves::core::GrpcClient client(cfg);

    auto start_status = client.Start();
    if (!start_status.ok()) {
        std::cerr << "Failed to start client: " << start_status << std::endl;
        return 1;
    }

    std::cout << "=== Norves Echo Client ===" << std::endl;
    std::cout << "Connecting to " << target << "..." << std::endl;

    auto conn_status = client.WaitForConnected(5000);
    if (!conn_status.ok()) {
        std::cerr << "Connection failed: " << conn_status << std::endl;
        client.Shutdown();
        return 1;
    }
    std::cout << "Connected!" << std::endl;

    auto stub = norves::core::CoreService::NewStub(client.GetGrpcChannel());

    // --- Ping ---
    {
        grpc::ClientContext ctx;
        norves::core::PingRequest req;
        norves::core::PingResponse resp;

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        req.set_timestamp(now);

        auto rpc_status = stub->Ping(&ctx, req, &resp);
        if (rpc_status.ok()) {
            auto rtt = resp.server_timestamp() - resp.client_timestamp();
            std::cout << "[Ping] RTT ~ " << rtt << " ms" << std::endl;
        } else {
            std::cerr << "[Ping] Failed: " << rpc_status.error_message() << std::endl;
        }
    }

    // --- Echo ---
    {
        grpc::ClientContext ctx;
        auto stream = stub->Echo(&ctx);

        std::cout << "\nType messages to echo (empty line to quit):" << std::endl;

        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) break;

            norves::core::EchoMessage msg;
            msg.set_content(line);
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            msg.set_timestamp(now);

            stream->Write(msg);

            norves::core::EchoMessage reply;
            if (stream->Read(&reply)) {
                std::cout << "  Echo: \"" << reply.content()
                          << "\" (server_ts=" << reply.timestamp() << ")"
                          << std::endl;
            }
        }

        stream->WritesDone();
        auto echo_status = stream->Finish();
        if (!echo_status.ok()) {
            std::cerr << "[Echo] Stream error: " << echo_status.error_message()
                      << std::endl;
        }
    }

    std::cout << "Shutting down client..." << std::endl;
    client.Shutdown();
    std::cout << "Done." << std::endl;

    return 0;
}
