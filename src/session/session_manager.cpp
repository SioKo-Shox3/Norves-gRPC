#include "norves/session/session_manager.h"

namespace norves::session {

SessionManager::SessionManager(std::pmr::memory_resource* memoryResource)
    : m_Sessions(memoryResource)
    , m_pMemoryResource(memoryResource)
{
}

void SessionManager::AddSession(std::shared_ptr<ISession> session)
{
    if (!session)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Sessions[session->GetId()] = std::move(session);
}

void SessionManager::RemoveSession(SessionId id)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Sessions.erase(id);
}

std::shared_ptr<ISession> SessionManager::GetSession(SessionId id) const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Sessions.find(id);
    if (it != m_Sessions.end())
    {
        return it->second;
    }
    return nullptr;
}

} // namespace norves::session
