#pragma once

#include <string>
#include <memory_resource>

namespace norves::session {

using SessionId = uint64_t;

/// Interface for a generic session.
class ISession
{
public:
    virtual ~ISession() = default;

    /// Returns the unique ID of this session.
    [[nodiscard]] virtual SessionId GetId() const noexcept = 0;

    /// Returns true if the session is currently connected.
    [[nodiscard]] virtual bool IsConnected() const noexcept = 0;

    /// Forcefully disconnects the session.
    virtual void Disconnect() = 0;
};

/// Implementation of a session.
class Session : public ISession
{
public:
    Session(SessionId id, std::pmr::memory_resource* memoryResource);
    ~Session() override = default;

    [[nodiscard]] SessionId GetId() const noexcept override
    {
        return m_Id;
    }

    [[nodiscard]] bool IsConnected() const noexcept override
    {
        return m_bConnected;
    }

    void Disconnect() override;

    /// Marks the session as connected.
    void SetConnected(bool bConnected);

private:
    SessionId m_Id;
    bool m_bConnected;
    std::pmr::memory_resource* m_pMemoryResource;
};

} // namespace norves::session
