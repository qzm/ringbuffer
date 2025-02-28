/**
 * @file example.cpp
 * @brief RingBuffer使用示例
 * @author Aaron Qiu
 * @date 2023-04-18
 */

#include "ringbuffer.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>

// 基本使用示例
void basicExample() {
    std::cout << "===== 基本使用示例 =====" << std::endl;
    
    // 创建一个容量为16的int类型RingBuffer
    RingBuffer<int, 16> buffer;
    
    // 写入数据
    for (int i = 0; i < 20; ++i) {
        bool success = buffer.Write(i);
        std::cout << "写入数据 " << i << ": " << (success ? "成功" : "失败") << std::endl;
    }
    
    // 读取数据
    int value;
    while (buffer.Read(value)) {
        std::cout << "读取数据: " << value << std::endl;
    }
    
    std::cout << std::endl;
}

// 移动语义示例
void moveExample() {
    std::cout << "===== 移动语义示例 =====" << std::endl;
    
    // 创建一个容量为8的string类型RingBuffer
    RingBuffer<std::string, 8> buffer;
    
    // 使用移动语义写入数据
    for (int i = 0; i < 5; ++i) {
        std::string str = "字符串 #" + std::to_string(i);
        std::cout << "原始字符串: " << str << std::endl;
        buffer.Write(std::move(str));
        std::cout << "移动后字符串: " << str << std::endl; // 应该为空或未定义
    }
    
    // 读取数据
    std::string value;
    while (buffer.Read(value)) {
        std::cout << "读取字符串: " << value << std::endl;
    }
    
    std::cout << std::endl;
}

// 批量操作示例
void batchExample() {
    std::cout << "===== 批量操作示例 =====" << std::endl;
    
    // 创建一个容量为64的int类型RingBuffer
    RingBuffer<int, 64> buffer;
    
    // 准备批量写入的数据
    std::vector<int> dataToWrite(20);
    for (int i = 0; i < 20; ++i) {
        dataToWrite[i] = i * 10;
    }
    
    // 批量写入
    size_t written = buffer.WriteBatch(dataToWrite.data(), dataToWrite.size());
    std::cout << "批量写入元素数量: " << written << "/" << dataToWrite.size() << std::endl;
    
    // 准备批量读取的缓冲区
    std::vector<int> dataRead(30);
    
    // 批量读取
    size_t read = buffer.ReadBatch(dataRead.data(), dataRead.size());
    std::cout << "批量读取元素数量: " << read << "/" << dataRead.size() << std::endl;
    
    // 打印读取的数据
    std::cout << "读取的数据: ";
    for (size_t i = 0; i < read; ++i) {
        std::cout << dataRead[i] << " ";
    }
    std::cout << std::endl << std::endl;
}

// 多线程示例
void multiThreadExample() {
    std::cout << "===== 多线程示例 =====" << std::endl;
    
    // 创建一个容量为128的int类型RingBuffer
    RingBuffer<int, 128> buffer;
    
    // 创建生产者线程
    std::thread producer([&buffer]() {
        for (int i = 0; i < 1000; ++i) {
            while (!buffer.Write(i)) {
                // 缓冲区已满，等待一小段时间
                std::this_thread::yield();
            }
            
            // 每写入100个元素休眠一小段时间，模拟生产者速度较慢的情况
            if (i % 100 == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        std::cout << "生产者线程完成" << std::endl;
    });
    
    // 创建消费者线程
    std::thread consumer([&buffer]() {
        int value;
        int count = 0;
        
        while (count < 1000) {
            if (buffer.Read(value)) {
                ++count;
                
                // 每读取10个元素打印一次状态
                if (count % 100 == 0) {
                    std::cout << "消费者已读取 " << count << " 个元素，当前值: " << value << std::endl;
                }
                
                // 每读取200个元素休眠一小段时间，模拟消费者速度较慢的情况
                if (count % 200 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            } else {
                // 缓冲区为空，等待一小段时间
                std::this_thread::yield();
            }
        }
        std::cout << "消费者线程完成" << std::endl;
    });
    
    // 等待线程完成
    producer.join();
    consumer.join();
    
    std::cout << std::endl;
}

// 辅助函数示例
void helperFunctionsExample() {
    std::cout << "===== 辅助函数示例 =====" << std::endl;
    
    // 创建一个容量为32的int类型RingBuffer
    RingBuffer<int, 32> buffer;
    
    // 检查初始状态
    std::cout << "初始容量: " << buffer.GetCapacity() << std::endl;
    std::cout << "初始大小: " << buffer.Size() << std::endl;
    std::cout << "是否为空: " << (buffer.IsEmpty() ? "是" : "否") << std::endl;
    std::cout << "是否已满: " << (buffer.IsFull() ? "是" : "否") << std::endl;
    
    // 写入一些数据
    for (int i = 0; i < 20; ++i) {
        buffer.Write(i);
    }
    
    // 检查写入后的状态
    std::cout << "写入后大小: " << buffer.Size() << std::endl;
    std::cout << "是否为空: " << (buffer.IsEmpty() ? "是" : "否") << std::endl;
    std::cout << "是否已满: " << (buffer.IsFull() ? "是" : "否") << std::endl;
    
    // 读取一些数据
    int value;
    for (int i = 0; i < 10; ++i) {
        buffer.Read(value);
    }
    
    // 检查读取后的状态
    std::cout << "读取后大小: " << buffer.Size() << std::endl;
    
    // 清空缓冲区
    buffer.Clear();
    
    // 检查清空后的状态
    std::cout << "清空后大小: " << buffer.Size() << std::endl;
    std::cout << "是否为空: " << (buffer.IsEmpty() ? "是" : "否") << std::endl;
    
    std::cout << std::endl;
}

int main() {
    std::cout << "RingBuffer 使用示例" << std::endl;
    std::cout << "===================" << std::endl << std::endl;
    
    // 运行各种示例
    basicExample();
    moveExample();
    batchExample();
    multiThreadExample();
    helperFunctionsExample();
    
    return 0;
} 