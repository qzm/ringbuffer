/**
 * @file ringbuffer.hpp
 * @brief RingBuffer implementation for high-performance circular buffer operations
 * @author Aaron Qiu
 * @date 2023-04-18
 */

#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <atomic>
#include <cassert>
#include <type_traits>

template <typename T, size_t Capacity>
class RingBuffer {
public:
    static_assert(Capacity > 0, "Capacity must be greater than 0.");

    RingBuffer() noexcept : readIndex_(0), writeIndex_(0) {}

    /**
     * @brief Write data to the RingBuffer
     * @param value The data to be written
     * @return Whether the write operation is successful
     */
    bool Write(const T& value) noexcept {
        const auto currentWrite = writeIndex_.load(std::memory_order_relaxed);
        const auto nextWrite = IncrementIndex(currentWrite);
        if (nextWrite == readIndex_.load(std::memory_order_acquire)) {
            // RingBuffer is full
            return false;
        }
        new (&buffer_[currentWrite]) T(value);
        writeIndex_.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * @brief Read data from the RingBuffer
     * @param value The read data
     * @return Whether the read operation is successful
     */
    bool Read(T& value) noexcept {
        const auto currentRead = readIndex_.load(std::memory_order_relaxed);
        if (currentRead == writeIndex_.load(std::memory_order_consume)) {
            // RingBuffer is empty
            return false;
        }
        value = std::move(*reinterpret_cast<T*>(&buffer_[currentRead]));
        reinterpret_cast<T*>(&buffer_[currentRead])->~T();
        readIndex_.store(IncrementIndex(currentRead), std::memory_order_release);
        return true;
    }

private:
    /**
     * @brief Increment the index value to achieve circular buffer behavior
     * @param index The current index value
     * @return The incremented index value
     */
    size_t IncrementIndex(size_t index) const noexcept {
        static_assert(std::is_unsigned_v<decltype(index)>, "Index must be unsigned integer.");
        static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2.");
        return (index + 1) & (Capacity - 1);
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> buffer_[Capacity]; // Buffer data
    alignas(64) std::atomic<size_t> readIndex_; // Read index
    alignas(64) std::atomic<size_t> writeIndex_; // Write index
};

#endif // RINGBUFFER_HPP
