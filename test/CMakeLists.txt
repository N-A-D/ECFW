cmake_minimum_required (VERSION 3.17)

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

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/)

package_add_test(type_traits ${SOURCE_DIR}/type_traits.cpp)
package_add_test(type_index ${SOURCE_DIR}/type_index.cpp)
package_add_test(sparse_set ${SOURCE_DIR}/sparse_set.cpp)
package_add_test(dword ${SOURCE_DIR}/dword.cpp)
package_add_test(world ${SOURCE_DIR}/world.cpp)
