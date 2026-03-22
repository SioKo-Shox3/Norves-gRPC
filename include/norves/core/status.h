#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <ostream>

namespace norves::core {

/// Status codes for Norves operations, abstracting gRPC status codes.
enum class StatusCode {
    Ok = 0,
    Cancelled,
    Unknown,
    InvalidArgument,
    DeadlineExceeded,
    NotFound,
    AlreadyExists,
    PermissionDenied,
    ResourceExhausted,
    Unavailable,
    Unimplemented,
    Internal,
    DataLoss,
    Unauthenticated,
};

/// Converts a StatusCode to a human-readable string.
[[nodiscard]] std::string_view StatusCodeToString(StatusCode code) noexcept;

/// Result of an operation -- either Ok or an error with a message.
class Status {
public:
    /// Constructs an OK status.
    Status() noexcept;

    /// Constructs a status with the given code and message.
    Status(StatusCode code, std::string message = {});

    /// Pre-built OK status.
    [[nodiscard]] static Status Ok() noexcept;

    /// Returns true if the status represents success.
    [[nodiscard]] bool ok() const noexcept { return code_ == StatusCode::Ok; }

    /// Returns true if the operation was cancelled.
    [[nodiscard]] bool is_cancelled() const noexcept { return code_ == StatusCode::Cancelled; }

    /// Returns the status code.
    [[nodiscard]] StatusCode code() const noexcept { return code_; }

    /// Returns the error message (empty if OK).
    [[nodiscard]] const std::string& message() const noexcept { return message_; }

    /// Returns a human-readable representation.
    [[nodiscard]] std::string ToString() const;

    /// Implicit bool conversion -- true if OK.
    explicit operator bool() const noexcept { return ok(); }

    friend std::ostream& operator<<(std::ostream& os, const Status& s);

private:
    StatusCode code_;
    std::string message_;
};

/// Converts a gRPC status code (int) to a Norves StatusCode.
[[nodiscard]] StatusCode FromGrpcStatusCode(int grpc_code) noexcept;

} // namespace norves::core
