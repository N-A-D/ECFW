cmake_minimum_required (VERSION 3.17)

project (ECFW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ECFW_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(ECFW_HEADER_PATH "${ECFW_PATH}/include")

add_library(${PROJECT_NAME} INTERFACE)
add_library(ECFW::ECFW ALIAS ECFW)
target_include_directories(
    ${PROJECT_NAME}
    INTERFACE 
    $<BUILD_INTERFACE:${ECFW_HEADER_PATH}>
    $<INSTALL_INTERFACE:include>
)

find_package(Boost 1.73.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE Boost::headers)
cmake_policy(SET CMP0074 NEW)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/ECFWConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS ECFW
    EXPORT ECFWTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ECFWConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/ECFWConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ECFW
)

install(EXPORT ECFWTargets 
    FILE ECFWTargets.cmake
    NAMESPACE ECFW::
    DESTINATION lib/cmake/ECFW)

install(FILES "${PROJECT_BINARY_DIR}/ECFWConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/ECFWConfig.cmake"
        DESTINATION lib/cmake/ECFW)

install(DIRECTORY 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    DESTINATION include)

option(BUILD_ECFW_TESTS ON)
if (${BUILD_ECFW_TESTS})
    enable_testing()
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")
endif()
