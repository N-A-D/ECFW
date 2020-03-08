cmake_minimum_required (VERSION 3.8)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("${PROJECT_SOURCE_DIR}/extern/google/googletest" "extern/google/googletest")

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${PROJECT_NAME})
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

set(TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(COMPONENT_TEST_DIR "${TEST_SRC_DIR}/component")
set(ENTITY_TEST_DIR "${TEST_SRC_DIR}/entity")

package_add_test(paged_vector_tests "${COMPONENT_TEST_DIR}/vector.cpp")
package_add_test(component_storage_tests "${COMPONENT_TEST_DIR}/storage.cpp")
package_add_test(entity_tests "${ENTITY_TEST_DIR}/entity.cpp")
package_add_test(entity_index_tests "${ENTITY_TEST_DIR}/index.cpp")