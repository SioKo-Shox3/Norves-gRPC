#include "norves/core/status.h"

#include <sstream>

namespace norves::core {

std::string_view StatusCodeToString(StatusCode code) noexcept
{
    switch (code)
    {
        case StatusCode::Ok:                return "OK";
        case StatusCode::Cancelled:         return "CANCELLED";
        case StatusCode::Unknown:           return "UNKNOWN";
        case StatusCode::InvalidArgument:   return "INVALID_ARGUMENT";
        case StatusCode::DeadlineExceeded:  return "DEADLINE_EXCEEDED";
        case StatusCode::NotFound:          return "NOT_FOUND";
        case StatusCode::AlreadyExists:     return "ALREADY_EXISTS";
        case StatusCode::PermissionDenied:  return "PERMISSION_DENIED";
        case StatusCode::ResourceExhausted: return "RESOURCE_EXHAUSTED";
        case StatusCode::Unavailable:       return "UNAVAILABLE";
        case StatusCode::Unimplemented:     return "UNIMPLEMENTED";
        case StatusCode::Internal:          return "INTERNAL";
        case StatusCode::DataLoss:          return "DATA_LOSS";
        case StatusCode::Unauthenticated:   return "UNAUTHENTICATED";
        default:                            return "UNKNOWN";
    }
}

Status::Status() noexcept : m_Code(StatusCode::Ok)
{
}

Status::Status(StatusCode code, std::pmr::string message)
    : m_Code(code), m_Message(std::move(message))
{
}

Status Status::Ok() noexcept
{
    return Status{};
}

std::string Status::ToString() const
{
    if (IsOk())
    {
        return "OK";
    }
    
    std::ostringstream oss;
    oss << StatusCodeToString(m_Code);
    if (!m_Message.empty())
    {
        oss << ": " << m_Message.c_str();
    }
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Status& s)
{
    return os << s.ToString();
}

StatusCode FromGrpcStatusCode(int grpc_code) noexcept
{
    switch (grpc_code)
    {
        case 0:  return StatusCode::Ok;
        case 1:  return StatusCode::Cancelled;
        case 2:  return StatusCode::Unknown;
        case 3:  return StatusCode::InvalidArgument;
        case 4:  return StatusCode::DeadlineExceeded;
        case 5:  return StatusCode::NotFound;
        case 6:  return StatusCode::AlreadyExists;
        case 7:  return StatusCode::PermissionDenied;
        case 8:  return StatusCode::ResourceExhausted;
        case 14: return StatusCode::Unavailable;
        case 12: return StatusCode::Unimplemented;
        case 13: return StatusCode::Internal;
        case 15: return StatusCode::DataLoss;
        case 16: return StatusCode::Unauthenticated;
        default: return StatusCode::Unknown;
    }
}

} // namespace norves::core
