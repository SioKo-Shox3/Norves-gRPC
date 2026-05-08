#pragma once

#include <vector>
#include <mutex>
#include <utility>
#include <memory_resource>

namespace norves::core {

/// A thread-safe, double-buffered queue optimized for single-consumer, multiple-producer scenarios
/// like a game engine network message queue.
template <typename T>
class DoubleBufferedQueue
{
public:
    explicit DoubleBufferedQueue(std::pmr::memory_resource* memoryResource = std::pmr::get_default_resource())
        : m_WriteBuffer(memoryResource)
        , m_ReadBuffer(memoryResource)
        , m_pMemoryResource(memoryResource)
    {
    }

    /// Enqueues an item safely from any thread.
    void Enqueue(T item)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_WriteBuffer.push_back(std::move(item));
    }

    /// Swaps the read and write buffers, outputting the contents of the write buffer.
    /// This should be called by the single consumer thread (e.g. the Game Tick thread).
    void SwapAndGet(std::pmr::vector<T>& outBuffer)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::swap(m_WriteBuffer, m_ReadBuffer);
        }
        
        outBuffer = std::move(m_ReadBuffer);
        m_ReadBuffer.clear();
    }

private:
    std::mutex m_Mutex;
    std::pmr::vector<T> m_WriteBuffer;
    std::pmr::vector<T> m_ReadBuffer;
    std::pmr::memory_resource* m_pMemoryResource;
};

} // namespace norves::core
