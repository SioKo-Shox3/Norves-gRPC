#pragma once

#include "norves/session/session.h"

#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <memory_resource>

namespace norves::session {

/// Manages a collection of active sessions.
class SessionManager
{
public:
    explicit SessionManager(std::pmr::memory_resource* memoryResource);
    ~SessionManager() = default;

    /// Adds a new session to the manager.
    void AddSession(std::shared_ptr<ISession> session);

    /// Removes a session by its ID.
    void RemoveSession(SessionId id);

    /// Retrieves a session by its ID.
    [[nodiscard]] std::shared_ptr<ISession> GetSession(SessionId id) const;

private:
    mutable std::mutex m_Mutex;
    
    std::pmr::unordered_map<SessionId, std::shared_ptr<ISession>> m_Sessions;
    
    std::pmr::memory_resource* m_pMemoryResource;
};

} // namespace norves::session
