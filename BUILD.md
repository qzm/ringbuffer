# 构建说明

本项目使用CMake构建系统。以下是构建步骤：

## 前提条件

- CMake 3.10或更高版本
- 支持C++17的编译器（GCC、Clang或MSVC）

## 基本构建步骤

### Linux/macOS

```bash
# 创建构建目录
mkdir build && cd build

# 配置
cmake ..

# 构建
cmake --build .

# 运行测试
ctest

# 运行示例程序
./test/example
./test/benchmark
```

### Windows

```cmd
# 创建构建目录
mkdir build
cd build

# 配置
cmake ..

# 构建
cmake --build . --config Release

# 运行测试
ctest -C Release

# 运行示例程序
.\test\Release\example.exe
.\test\Release\benchmark.exe
```

## 高性能构建

本项目提供了多种高级性能优化选项，可以显著提高程序性能。

### 性能分析引导优化(PGO)

PGO是一种两阶段构建技术，通过收集程序实际运行时的性能数据来指导编译器优化。

#### Linux/macOS

```bash
# 第一阶段：生成性能分析数据
mkdir build_pgo && cd build_pgo
cmake -DUSE_PGO=ON -DPGO_GENERATE=ON ..
cmake --build .
make benchmark_run  # 运行程序生成性能数据

# 第二阶段：使用性能分析数据进行优化构建
cmake -DUSE_PGO=ON -DPGO_GENERATE=OFF -DPGO_USE=ON ..
cmake --build .
```

#### Windows

```cmd
# 第一阶段：生成性能分析数据
mkdir build_pgo
cd build_pgo
cmake -DUSE_PGO=ON -DPGO_GENERATE=ON ..
cmake --build . --config Release
cmake --build . --target benchmark_run  # 运行程序生成性能数据

# 第二阶段：使用性能分析数据进行优化构建
cmake -DUSE_PGO=ON -DPGO_GENERATE=OFF -DPGO_USE=ON ..
cmake --build . --config Release
```

### 自定义优化选项

您可以通过CMake选项自定义优化级别：

```bash
# 示例：禁用AVX指令集优化
cmake -DUSE_AVX=OFF ..

# 示例：启用更激进的优化（可能影响浮点精度）
cmake -DAGGRESSIVE_OPTIMIZATION=ON ..
```

## 百万级数据性能测试

本项目支持百万级数据的性能测试，可以评估RingBuffer在大数据量下的性能表现。

### 运行百万级数据测试

```bash
# Linux/macOS
mkdir build && cd build
cmake ..
cmake --build .
./test/benchmark

# Windows
mkdir build
cd build
cmake ..
cmake --build . --config Release
.\test\Release\benchmark.exe
```

### 使用PGO优化百万级数据测试

为了获得最佳性能，建议使用PGO优化后再进行百万级数据测试：

```bash
# Linux/macOS - 第一阶段
mkdir build_pgo && cd build_pgo
cmake -DUSE_PGO=ON -DPGO_GENERATE=ON ..
cmake --build .
make benchmark_run  # 生成性能分析数据

# Linux/macOS - 第二阶段
cmake -DUSE_PGO=ON -DPGO_GENERATE=OFF -DPGO_USE=ON ..
cmake --build .
./test/benchmark  # 运行优化后的测试

# Windows - 第一阶段
mkdir build_pgo
cd build_pgo
cmake -DUSE_PGO=ON -DPGO_GENERATE=ON ..
cmake --build . --config Release
cmake --build . --target benchmark_run

# Windows - 第二阶段
cmake -DUSE_PGO=ON -DPGO_GENERATE=OFF -DPGO_USE=ON ..
cmake --build . --config Release
.\test\Release\benchmark.exe  # 运行优化后的测试
```

### 百万级数据测试配置

百万级数据测试使用以下配置：

- 缓冲区容量：1,048,576 (1M)
- 单线程测试：1亿次操作
- 批量操作测试：每种批量大小处理100万个元素
- 多线程测试：5千万次操作
- 大数据量批量测试：每次处理100万元素，重复10次

如果需要调整测试参数，可以修改`test/benchmark.cpp`文件中的相关常量。

## 安装

```bash
# Linux/macOS
cmake --build . --target install

# Windows (以管理员身份运行)
cmake --build . --config Release --target install
```

## 使用CMake集成到其他项目

如果您想在自己的CMake项目中使用RingBuffer，可以这样做：

```cmake
# 在您的CMakeLists.txt中
find_path(RINGBUFFER_INCLUDE_DIR ringbuffer.hpp)
include_directories(${RINGBUFFER_INCLUDE_DIR})

# 然后在您的代码中
#include "ringbuffer.hpp"
```

或者直接将ringbuffer.hpp复制到您的项目中使用。

## 更多优化信息

有关更多高级性能优化选项和技术，请参考[cmake/README.md](cmake/README.md)文件。 