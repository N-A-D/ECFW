cmake_minimum_required (VERSION 3.8)

project (libECFW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ECFW_PATH "${PROJECT_SOURCE_DIR}")
set(ECFW_HEADER_PATH "${ECFW_PATH}/include")

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
	${PROJECT_NAME}
	INTERFACE
	$<BUILD_INTERFACE:${ECFW_HEADER_PATH}>
	$<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/libECFWConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS libECFW
    EXPORT libECFWTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/libECFWConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/libECFWConfig.cmake"
    INSTALL_DESTINATION lib/cmake/libECFW
)

install(EXPORT libECFWTargets DESTINATION lib/cmake/libECFW)
install(FILES "${PROJECT_BINARY_DIR}/libECFWConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/libECFWConfig.cmake"
        DESTINATION lib/cmake/libECFW)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

set(HANA_PATH "${PROJECT_SOURCE_DIR}/extern/boost/hana")
set(HANA_HEADER_PATH "${HANA_PATH}/include")

add_subdirectory(${HANA_PATH})
target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${HANA_HEADER_PATH}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME} INTERFACE hana)

option(BUILD_ECFW_TESTS ON)
if (${BUILD_ECFW_TESTS})
    enable_testing()
    add_subdirectory("${PROJECT_SOURCE_DIR}/test")
endif()
