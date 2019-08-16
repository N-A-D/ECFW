cmake_minimum_required(VERSION 3.8)

add_subdirectory("${PROJECT_SOURCE_DIR}/extern/google/benchmark" "extern/google/benchmark")

add_executable(bench benchmarks.cpp)

target_link_libraries(
	bench
	${PROJECT_NAME}
	benchmark
	benchmark_main
)