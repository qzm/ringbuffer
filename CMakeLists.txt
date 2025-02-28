cmake_minimum_required(VERSION 3.10)
project(RingBuffer VERSION 2.0.0 LANGUAGES CXX)

# 添加cmake模块路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 包含优化模块
include(Optimization)
include(PGOSupport)
include(CacheOptimization)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型，如果未指定则默认为Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")

# 启用高性能优化
enable_high_performance_optimizations()

# 打印PGO状态
print_pgo_status()

# 设置头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 启用测试
enable_testing()

# 添加测试子目录
add_subdirectory(test)

# 安装头文件
install(FILES ringbuffer.hpp DESTINATION include) 