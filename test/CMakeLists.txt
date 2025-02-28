# 测试程序的CMake配置

# 添加测试可执行文件
add_executable(example example.cpp)
add_executable(benchmark benchmark.cpp)

# 设置包含目录
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(benchmark PRIVATE ${CMAKE_SOURCE_DIR})

# 为基准测试程序应用所有优化
enable_all_optimizations_for_target(benchmark)

# 为基准测试程序添加缓存优化
add_cache_optimization_for_target(benchmark)

# 为基准测试程序添加假共享优化
add_false_sharing_optimization_for_target(benchmark)

# 为基准测试程序配置PGO
configure_pgo_for_target(benchmark)
add_pgo_targets(benchmark)

# 链接线程库
find_package(Threads REQUIRED)
target_link_libraries(example PRIVATE Threads::Threads)
target_link_libraries(benchmark PRIVATE Threads::Threads)

# 如果支持，添加数学库优化
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(benchmark PRIVATE ${MATH_LIBRARY})
endif()

# 添加测试
add_test(NAME example_test COMMAND example)
add_test(NAME benchmark_test COMMAND benchmark)

# 设置基准测试的环境变量，提高性能
set_tests_properties(benchmark_test PROPERTIES
    ENVIRONMENT "OMP_NUM_THREADS=2;OMP_PROC_BIND=true;OMP_PLACES=cores"
)
