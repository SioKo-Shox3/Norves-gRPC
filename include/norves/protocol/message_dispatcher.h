#pragma once

#include <functional>
#include <unordered_map>
#include <memory_resource>
#include <string>
#include <typeindex>

namespace norves::protocol {

/// Dispatches messages to registered callbacks based on their type index.
template <typename MessageBaseT, typename ContextT>
class MessageDispatcher
{
public:
    using MessageHandler = std::function<void(const MessageBaseT&, ContextT&)>;

    explicit MessageDispatcher(std::pmr::memory_resource* memoryResource = std::pmr::get_default_resource())
        : m_Handlers(memoryResource)
        , m_pMemoryResource(memoryResource)
    {
    }

    /// Registers a handler for a specific message type.
    template <typename SpecificMessageT>
    void RegisterHandler(std::function<void(const SpecificMessageT&, ContextT&)> handler)
    {
        auto typeIdx = std::type_index(typeid(SpecificMessageT));
        m_Handlers[typeIdx] = [handler](const MessageBaseT& baseMsg, ContextT& context) {
            const auto& specificMsg = static_cast<const SpecificMessageT&>(baseMsg);
            handler(specificMsg, context);
        };
    }

    /// Dispatches a message to its registered handler.
    bool Dispatch(const MessageBaseT& msg, ContextT& context) const
    {
        auto typeIdx = std::type_index(typeid(msg));
        auto it = m_Handlers.find(typeIdx);
        if (it != m_Handlers.end())
        {
            it->second(msg, context);
            return true;
        }
        return false;
    }

private:
    std::pmr::unordered_map<std::type_index, MessageHandler> m_Handlers;
    std::pmr::memory_resource* m_pMemoryResource;
};

} // namespace norves::protocol
