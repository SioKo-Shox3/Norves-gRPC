#include "norves/session/session.h"

namespace norves::session {

Session::Session(SessionId id, std::pmr::memory_resource* memoryResource)
    : m_Id(id)
    , m_bConnected(false)
    , m_pMemoryResource(memoryResource)
{
}

void Session::Disconnect()
{
    m_bConnected = false;
    // In a real implementation, this would signal the underlying stream handler to close.
}

void Session::SetConnected(bool bConnected)
{
    m_bConnected = bConnected;
}

} // namespace norves::session
