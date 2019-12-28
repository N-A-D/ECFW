cmake_minimum_required (VERSION 3.8)

project (ecfw)

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type provided, default is Debug.")
	set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ECFW_HEADER_PATH ${CMAKE_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
	${PROJECT_NAME} INTERFACE
	${ECFW_HEADER_PATH}
)

set(ECFW_HEADER_FILES
	${ECFW_HEADER_PATH}/ecfw/component/mask.hpp
	${ECFW_HEADER_PATH}/ecfw/component/storage.hpp
	${ECFW_HEADER_PATH}/ecfw/entity/events.hpp
	${ECFW_HEADER_PATH}/ecfw/entity/group.hpp
	${ECFW_HEADER_PATH}/ecfw/entity/manager.hpp
	${ECFW_HEADER_PATH}/ecfw/entity/traits.hpp
	${ECFW_HEADER_PATH}/ecfw/event/dispatcher.hpp
	${ECFW_HEADER_PATH}/ecfw/event/signal.hpp
	${ECFW_HEADER_PATH}/ecfw/meta/type_list.hpp
	${ECFW_HEADER_PATH}/ecfw/meta/type_traits.hpp
	${ECFW_HEADER_PATH}/ecfw/utility/noncopyable.hpp
	${ECFW_HEADER_PATH}/ecfw/ecfw.hpp
)

add_custom_target(ECFW_SOURCES SOURCES ${ECFW_HEADER_FILES})

add_subdirectory(extern/proto)
target_link_libraries(${PROJECT_NAME} INTERFACE proto)

option(BUILD_ECFW_TESTS "Build ECFW tests" OFF)

if (BUILD_ECFW_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

option(BUILD_ECFW_BENCHMARKS "Build ECFW benchmarks" OFF)

if (BUILD_ECFW_BENCHMARKS)
	add_subdirectory(benchmark)
endif()