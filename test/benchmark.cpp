/**
 * @file benchmark.cpp
 * @brief RingBuffer性能测试程序
 * @author Aaron Qiu
 * @date 2023-04-18
 */

#include "ringbuffer.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <iomanip>

// 单线程性能测试
template <typename T, size_t Capacity>
void singleThreadBenchmark() {
    RingBuffer<T, Capacity> buffer;
    const int iterations = 10000000; // 1千万次操作
    
    std::cout << "===== 单线程性能测试 (缓冲区容量: " << Capacity << ") =====" << std::endl;
    
    // 单元素写入测试
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            buffer.Write(static_cast<T>(i));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
        std::cout << "单元素写入: " << std::fixed << std::setprecision(2) 
                  << static_cast<double>(duration) / iterations << " 纳秒/操作" << std::endl;
    }
    
    // 单元素读取测试
    {
        T value;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            buffer.Read(value);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
        std::cout << "单元素读取: " << std::fixed << std::setprecision(2) 
                  << static_cast<double>(duration) / iterations << " 纳秒/操作" << std::endl;
    }
}

// 批量操作性能测试
template <typename T, size_t Capacity, size_t BatchSize>
void batchBenchmark() {
    RingBuffer<T, Capacity> buffer;
    const int iterations = 1000000 / BatchSize; // 总共处理100万个元素
    
    std::cout << "===== 批量操作性能测试 (缓冲区容量: " << Capacity << ", 批量大小: " << BatchSize << ") =====" << std::endl;
    
    // 准备批量写入的数据
    std::vector<T> dataToWrite(BatchSize);
    for (size_t i = 0; i < BatchSize; ++i) {
        dataToWrite[i] = static_cast<T>(i);
    }
    
    // 准备批量读取的缓冲区
    std::vector<T> dataRead(BatchSize);
    
    // 批量写入测试
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            buffer.WriteBatch(dataToWrite.data(), BatchSize);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
        std::cout << "批量写入: " << std::fixed << std::setprecision(2) 
                  << static_cast<double>(duration) / (iterations * BatchSize) << " 纳秒/元素" << std::endl;
    }
    
    // 批量读取测试
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            buffer.ReadBatch(dataRead.data(), BatchSize);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
        std::cout << "批量读取: " << std::fixed << std::setprecision(2) 
                  << static_cast<double>(duration) / (iterations * BatchSize) << " 纳秒/元素" << std::endl;
    }
}

// 多线程性能测试（单生产者单消费者）
template <typename T, size_t Capacity>
void multiThreadBenchmark() {
    RingBuffer<T, Capacity> buffer;
    const int iterations = 10000000; // 1千万次操作
    std::atomic<bool> producerDone(false);
    std::atomic<int> consumed(0);
    
    std::cout << "===== 多线程性能测试 (缓冲区容量: " << Capacity << ", 单生产者单消费者) =====" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 生产者线程
    std::thread producer([&]() {
        for (int i = 0; i < iterations; ++i) {
            while (!buffer.Write(static_cast<T>(i))) {
                std::this_thread::yield();
            }
        }
        producerDone.store(true, std::memory_order_release);
    });
    
    // 消费者线程
    std::thread consumer([&]() {
        T value;
        while (consumed.load(std::memory_order_relaxed) < iterations || !producerDone.load(std::memory_order_acquire)) {
            if (buffer.Read(value)) {
                consumed.fetch_add(1, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "多线程读写: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(duration) / (iterations * 2) << " 纳秒/操作" << std::endl;
}

int main() {
    std::cout << "RingBuffer 性能测试" << std::endl;
    std::cout << "==================" << std::endl;
    
    // 单线程测试 - 使用更大的缓冲区
    singleThreadBenchmark<int, 1048576>(); // 1M容量
    
    // 批量操作测试 - 使用更大的缓冲区
    batchBenchmark<int, 1048576, 10>();
    batchBenchmark<int, 1048576, 100>();
    batchBenchmark<int, 1048576, 1000>();
    batchBenchmark<int, 1048576, 10000>();
    
    // 多线程测试 - 使用更大的缓冲区
    multiThreadBenchmark<int, 1048576>();
    
    return 0;
} 