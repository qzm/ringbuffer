/**
 * @file ringbuffer.hpp
 * @brief RingBuffer implementation for high-performance circular buffer operations
 * @author Aaron Qiu
 * @date 2023-04-18
 * @version 2.0.0
 */

#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <atomic>
#include <cassert>
#include <type_traits>
#include <cstddef>

// 预取指令支持
#if defined(__x86_64__) || defined(_M_X64)
    #include <immintrin.h>
    #define PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
#elif defined(__aarch64__) || defined(_M_ARM64)
    #include <arm_neon.h>
    #define PREFETCH(addr) __builtin_prefetch(addr, 0, 3)
#else
    #define PREFETCH(addr)
#endif

// 强制内联
#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#else
    #define FORCE_INLINE inline __attribute__((always_inline))
#endif

// 分支预测提示
#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x) __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
#endif

template <typename T, size_t Capacity>
class RingBuffer {
public:
    static_assert(Capacity > 0, "Capacity must be greater than 0.");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2.");
    static_assert(Capacity <= 1024*1024*1024, "Capacity must be less than or equal to 1GB.");

    RingBuffer() noexcept : readIndex_(0), writeIndex_(0) {}

    /**
     * @brief Write data to the RingBuffer
     * @param value The data to be written
     * @return Whether the write operation is successful
     */
    FORCE_INLINE bool Write(const T& value) noexcept {
        const auto currentWrite = writeIndex_.load(std::memory_order_relaxed);
        const auto nextWrite = IncrementIndex(currentWrite);
        
        // 预取下一个写入位置的缓存行
        PREFETCH(&buffer_[nextWrite]);
        
        if (UNLIKELY(nextWrite == readIndex_.load(std::memory_order_acquire))) {
            // RingBuffer is full
            return false;
        }
        
        new (&buffer_[currentWrite]) T(value);
        writeIndex_.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * @brief Write data to the RingBuffer (move semantics)
     * @param value The data to be written
     * @return Whether the write operation is successful
     */
    FORCE_INLINE bool Write(T&& value) noexcept {
        const auto currentWrite = writeIndex_.load(std::memory_order_relaxed);
        const auto nextWrite = IncrementIndex(currentWrite);
        
        // 预取下一个写入位置的缓存行
        PREFETCH(&buffer_[nextWrite]);
        
        if (UNLIKELY(nextWrite == readIndex_.load(std::memory_order_acquire))) {
            // RingBuffer is full
            return false;
        }
        
        new (&buffer_[currentWrite]) T(std::move(value));
        writeIndex_.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * @brief Read data from the RingBuffer
     * @param value The read data
     * @return Whether the read operation is successful
     */
    FORCE_INLINE bool Read(T& value) noexcept {
        const auto currentRead = readIndex_.load(std::memory_order_relaxed);
        
        if (UNLIKELY(currentRead == writeIndex_.load(std::memory_order_acquire))) {
            // RingBuffer is empty
            return false;
        }
        
        // 预取下一个读取位置的缓存行
        const auto nextRead = IncrementIndex(currentRead);
        PREFETCH(&buffer_[nextRead]);
        
        value = std::move(*reinterpret_cast<T*>(&buffer_[currentRead]));
        reinterpret_cast<T*>(&buffer_[currentRead])->~T();
        readIndex_.store(nextRead, std::memory_order_release);
        return true;
    }

    /**
     * @brief Batch write multiple elements to the RingBuffer
     * @param values Pointer to the array of values to write
     * @param count Number of elements to write
     * @return Number of elements successfully written
     */
    FORCE_INLINE size_t WriteBatch(const T* values, size_t count) noexcept {
        size_t written = 0;
        const auto initialWrite = writeIndex_.load(std::memory_order_relaxed);
        auto currentWrite = initialWrite;
        
        while (written < count) {
            const auto nextWrite = IncrementIndex(currentWrite);
            if (nextWrite == readIndex_.load(std::memory_order_acquire)) {
                // RingBuffer is full
                break;
            }
            
            // 预取后续数据
            if (written + 1 < count) {
                PREFETCH(&values[written + 1]);
            }
            
            new (&buffer_[currentWrite]) T(values[written]);
            currentWrite = nextWrite;
            ++written;
        }
        
        if (written > 0) {
            writeIndex_.store(currentWrite, std::memory_order_release);
        }
        
        return written;
    }

    /**
     * @brief Batch read multiple elements from the RingBuffer
     * @param values Pointer to the array where read values will be stored
     * @param count Maximum number of elements to read
     * @return Number of elements successfully read
     */
    FORCE_INLINE size_t ReadBatch(T* values, size_t count) noexcept {
        size_t read = 0;
        const auto initialRead = readIndex_.load(std::memory_order_relaxed);
        auto currentRead = initialRead;
        
        while (read < count) {
            if (currentRead == writeIndex_.load(std::memory_order_acquire)) {
                // RingBuffer is empty
                break;
            }
            
            // 预取后续数据
            if (read + 1 < count) {
                const auto nextNextRead = IncrementIndex(IncrementIndex(currentRead));
                PREFETCH(&buffer_[nextNextRead]);
            }
            
            values[read] = std::move(*reinterpret_cast<T*>(&buffer_[currentRead]));
            reinterpret_cast<T*>(&buffer_[currentRead])->~T();
            currentRead = IncrementIndex(currentRead);
            ++read;
        }
        
        if (read > 0) {
            readIndex_.store(currentRead, std::memory_order_release);
        }
        
        return read;
    }

    /**
     * @brief Get the current size of the RingBuffer
     * @return Current number of elements in the RingBuffer
     */
    FORCE_INLINE size_t Size() const noexcept {
        const auto read = readIndex_.load(std::memory_order_relaxed);
        const auto write = writeIndex_.load(std::memory_order_relaxed);
        return write >= read ? write - read : Capacity - (read - write);
    }

    /**
     * @brief Check if the RingBuffer is empty
     * @return True if empty, false otherwise
     */
    FORCE_INLINE bool IsEmpty() const noexcept {
        return readIndex_.load(std::memory_order_relaxed) == 
               writeIndex_.load(std::memory_order_relaxed);
    }

    /**
     * @brief Check if the RingBuffer is full
     * @return True if full, false otherwise
     */
    FORCE_INLINE bool IsFull() const noexcept {
        const auto nextWrite = IncrementIndex(writeIndex_.load(std::memory_order_relaxed));
        return nextWrite == readIndex_.load(std::memory_order_relaxed);
    }

    /**
     * @brief Get the capacity of the RingBuffer
     * @return Maximum number of elements the RingBuffer can hold
     */
    FORCE_INLINE constexpr size_t GetCapacity() const noexcept {
        return Capacity;
    }

    /**
     * @brief Clear the RingBuffer
     */
    void Clear() noexcept {
        auto current = readIndex_.load(std::memory_order_relaxed);
        const auto end = writeIndex_.load(std::memory_order_relaxed);
        
        while (current != end) {
            reinterpret_cast<T*>(&buffer_[current])->~T();
            current = IncrementIndex(current);
        }
        
        readIndex_.store(0, std::memory_order_relaxed);
        writeIndex_.store(0, std::memory_order_relaxed);
    }

private:
    /**
     * @brief Increment the index value to achieve circular buffer behavior
     * @param index The current index value
     * @return The incremented index value
     */
    FORCE_INLINE size_t IncrementIndex(size_t index) const noexcept {
        static_assert(std::is_unsigned_v<decltype(index)>, "Index must be unsigned integer.");
        return (index + 1) & (Capacity - 1);
    }

private:
    // 使用缓存行对齐以避免伪共享
    alignas(128) std::aligned_storage_t<sizeof(T), alignof(T)> buffer_[Capacity]; // Buffer data
    alignas(128) std::atomic<size_t> readIndex_; // Read index
    alignas(128) std::atomic<size_t> writeIndex_; // Write index
};

#endif // RINGBUFFER_HPP
