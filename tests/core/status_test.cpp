#include "norves/core/status.h"

#include <gtest/gtest.h>

using norves::core::Status;
using norves::core::StatusCode;
using norves::core::StatusCodeToString;
using norves::core::FromGrpcStatusCode;

TEST(StatusTest, DefaultConstructorIsOk) {
    Status s;
    EXPECT_TRUE(s.ok());
    EXPECT_EQ(s.code(), StatusCode::Ok);
    EXPECT_TRUE(s.message().empty());
}

TEST(StatusTest, OkFactoryMethod) {
    auto s = Status::Ok();
    EXPECT_TRUE(s.ok());
    EXPECT_TRUE(static_cast<bool>(s));
}

TEST(StatusTest, ErrorStatus) {
    Status s{StatusCode::NotFound, "resource not found"};
    EXPECT_FALSE(s.ok());
    EXPECT_EQ(s.code(), StatusCode::NotFound);
    EXPECT_EQ(s.message(), "resource not found");
    EXPECT_FALSE(static_cast<bool>(s));
}

TEST(StatusTest, IsCancelled) {
    Status s{StatusCode::Cancelled, "user cancelled"};
    EXPECT_TRUE(s.is_cancelled());
}

TEST(StatusTest, ToStringOk) {
    EXPECT_EQ(Status::Ok().ToString(), "OK");
}

TEST(StatusTest, ToStringError) {
    Status s{StatusCode::Internal, "something broke"};
    EXPECT_EQ(s.ToString(), "INTERNAL: something broke");
}

TEST(StatusTest, ToStringErrorNoMessage) {
    Status s{StatusCode::Unavailable};
    EXPECT_EQ(s.ToString(), "UNAVAILABLE");
}

TEST(StatusCodeToStringTest, AllCodes) {
    EXPECT_EQ(StatusCodeToString(StatusCode::Ok), "OK");
    EXPECT_EQ(StatusCodeToString(StatusCode::Cancelled), "CANCELLED");
    EXPECT_EQ(StatusCodeToString(StatusCode::InvalidArgument), "INVALID_ARGUMENT");
    EXPECT_EQ(StatusCodeToString(StatusCode::DeadlineExceeded), "DEADLINE_EXCEEDED");
    EXPECT_EQ(StatusCodeToString(StatusCode::Internal), "INTERNAL");
}

TEST(FromGrpcStatusCodeTest, MapsCorrectly) {
    EXPECT_EQ(FromGrpcStatusCode(0), StatusCode::Ok);
    EXPECT_EQ(FromGrpcStatusCode(1), StatusCode::Cancelled);
    EXPECT_EQ(FromGrpcStatusCode(3), StatusCode::InvalidArgument);
    EXPECT_EQ(FromGrpcStatusCode(5), StatusCode::NotFound);
    EXPECT_EQ(FromGrpcStatusCode(14), StatusCode::Unavailable);
    EXPECT_EQ(FromGrpcStatusCode(999), StatusCode::Unknown);
}
