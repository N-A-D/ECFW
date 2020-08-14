cmake_minimum_required(VERSION 3.17)

set(CMAKE_BUILD_TYPE "Release")
set(BENCHMARK_ENABLE_LTO ON)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)
set(BENCHMARK_ENABLE_TESTING OFF)

set(BENCHMARK_NAME "ecfw_benchmarks")
add_executable("${BENCHMARK_NAME}" src/benchmark.cpp)
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/google/benchmark" "extern/google/benchmark")
target_link_libraries("${BENCHMARK_NAME}" PRIVATE benchmark::benchmark benchmark::benchmark_main)
target_link_libraries("${BENCHMARK_NAME}" PRIVATE ECFW::ECFW)