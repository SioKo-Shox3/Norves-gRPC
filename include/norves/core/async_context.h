#pragma once

namespace norves::core {

/// Interface for any asynchronous operation queued in the CompletionQueue.
class IAsyncContext
{
public:
    virtual ~IAsyncContext() = default;

    /// Called by the CompletionQueue polling loop when an event completes.
    /// @param bOk True if the operation succeeded, false otherwise.
    virtual void OnEvent(bool bOk) = 0;
};

} // namespace norves::core
