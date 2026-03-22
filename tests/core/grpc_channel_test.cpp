#include "norves/core/grpc_channel.h"

#include <gtest/gtest.h>

using norves::core::GrpcChannel;
using norves::core::ChannelConfig;
using norves::core::StatusCode;

TEST(GrpcChannelTest, ConstructsWithConfig) {
    ChannelConfig cfg;
    cfg.target = "localhost:50051";
    cfg.use_tls = false;

    GrpcChannel ch(cfg);

    EXPECT_EQ(ch.target(), "localhost:50051");
    EXPECT_NE(ch.GetChannel(), nullptr);
}

TEST(GrpcChannelTest, IsNotConnectedWithoutServer) {
    ChannelConfig cfg;
    cfg.target = "localhost:19999";

    GrpcChannel ch(cfg);

    EXPECT_FALSE(ch.IsConnected());
}

TEST(GrpcChannelTest, WaitForConnectedTimesOut) {
    ChannelConfig cfg;
    cfg.target = "localhost:19999";

    GrpcChannel ch(cfg);

    auto status = ch.WaitForConnected(500);
    EXPECT_EQ(status.code(), StatusCode::DeadlineExceeded);
}

TEST(GrpcChannelTest, MoveConstructor) {
    ChannelConfig cfg;
    cfg.target = "localhost:50051";

    GrpcChannel ch1(cfg);
    GrpcChannel ch2(std::move(ch1));

    EXPECT_EQ(ch2.target(), "localhost:50051");
    EXPECT_NE(ch2.GetChannel(), nullptr);
}
