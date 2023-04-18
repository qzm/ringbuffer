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

template <typename T, size_t Capacity>
class RingBuffer {
public:
    RingBuffer() : readIndex_(0), writeIndex_(0) {}

    /**
     * @brief Write data to the RingBuffer
     * @param value The data to be written
     * @return Whether the write operation is successful
     */
    bool Write(const T& value) {
        const auto currentWrite = writeIndex_.load(std::memory_order_relaxed);
        const auto nextWrite = IncrementIndex(currentWrite);
        if (nextWrite == readIndex_.load(std::memory_order_acquire)) {
            // RingBuffer is full
            return false;
        }
        buffer_[currentWrite] = value;
        writeIndex_.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * @brief Read data from the RingBuffer
     * @param value The read data
     * @return Whether the read operation is successful
     */
    bool Read(T& value) {
        const auto currentRead = readIndex_.load(std::memory_order_relaxed);
        if (currentRead == writeIndex_.load(std::memory_order_acquire)) {
            // RingBuffer is empty
            return false;
        }
        value = buffer_[currentRead];
        readIndex_.store(IncrementIndex(currentRead), std::memory_order_release);
        return true;
    }

private:
    /**
     * @brief Increment the index value to achieve circular buffer behavior
     * @param index The current index value
     * @return The incremented index value
     */
    size_t IncrementIndex(size_t index) const {
        assert(index < Capacity);
        return (index + 1) % Capacity;
    }

private:
    alignas(64) T buffer_[Capacity]; // Buffer data
    alignas(64) std::atomic<size_t> readIndex_; // Read index
    alignas(64) std::atomic<size_t> writeIndex_; // Write index
};

#endif // RINGBUFFER_HPP
