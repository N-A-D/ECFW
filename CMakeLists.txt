cmake_minimum_required (VERSION 3.8)

project (ecfw)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
	${PROJECT_NAME}
	INTERFACE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

add_subdirectory(extern/ancillary)
target_link_libraries(${PROJECT_NAME} INTERFACE ancillary)

add_subdirectory(extern/proto)
target_link_libraries(${PROJECT_NAME} INTERFACE proto)

option(BUILD_ECFW_TESTS "Build ECFW tests" ON)
if (BUILD_ECFW_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
