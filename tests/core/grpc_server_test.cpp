#include "norves/core/grpc_server.h"

#include <gtest/gtest.h>

using norves::core::GrpcServer;
using norves::core::ServerConfig;
using norves::core::StatusCode;

TEST(GrpcServerTest, StartsAndStops) {
    ServerConfig cfg;
    cfg.listen_address = "127.0.0.1:0";
    cfg.num_cq_threads = 1;

    GrpcServer server(cfg);

    auto status = server.Start();
    ASSERT_TRUE(status.ok()) << status.ToString();
    EXPECT_TRUE(server.IsRunning());
    EXPECT_GT(server.port(), 0);

    server.Shutdown();
    EXPECT_FALSE(server.IsRunning());
}

TEST(GrpcServerTest, DoubleStartReturnsError) {
    ServerConfig cfg;
    cfg.listen_address = "127.0.0.1:0";

    GrpcServer server(cfg);
    ASSERT_TRUE(server.Start().ok());

    auto status = server.Start();
    EXPECT_EQ(status.code(), StatusCode::AlreadyExists);

    server.Shutdown();
}

TEST(GrpcServerTest, ShutdownIdempotent) {
    ServerConfig cfg;
    cfg.listen_address = "127.0.0.1:0";

    GrpcServer server(cfg);
    ASSERT_TRUE(server.Start().ok());

    server.Shutdown();
    server.Shutdown();
    EXPECT_FALSE(server.IsRunning());
}

TEST(GrpcServerTest, DestructorCallsShutdown) {
    ServerConfig cfg;
    cfg.listen_address = "127.0.0.1:0";

    {
        GrpcServer server(cfg);
        ASSERT_TRUE(server.Start().ok());
    }
}
