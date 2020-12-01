cmake_minimum_required (VERSION 3.17)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(
    "${PROJECT_SOURCE_DIR}/extern/google/googletest" "extern/google/googletest")

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

set(TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(DETAIL_SOURCE_DIR ${TEST_SOURCE_DIR}/detail)
package_add_test(detail_entity ${DETAIL_SOURCE_DIR}/entity.cpp)
package_add_test(detail_sparse_set ${DETAIL_SOURCE_DIR}/sparse_set.cpp)
package_add_test(detail_type_index ${DETAIL_SOURCE_DIR}/type_index.cpp)
package_add_test(detail_type_list ${DETAIL_SOURCE_DIR}/type_list.cpp)

set(WORLD_SOURCE_DIR ${TEST_SOURCE_DIR}/world)
package_add_test(world_create ${WORLD_SOURCE_DIR}/create.cpp)
package_add_test(world_clone ${WORLD_SOURCE_DIR}/clone.cpp)
package_add_test(world_recycle ${WORLD_SOURCE_DIR}/recycle.cpp)
package_add_test(world_orphan ${WORLD_SOURCE_DIR}/orphan.cpp)
package_add_test(world_destroy ${WORLD_SOURCE_DIR}/destroy.cpp)
package_add_test(world_get ${WORLD_SOURCE_DIR}/get.cpp)
package_add_test(world_has ${WORLD_SOURCE_DIR}/has.cpp)
package_add_test(world_assign ${WORLD_SOURCE_DIR}/assign.cpp)
package_add_test(world_remove ${WORLD_SOURCE_DIR}/remove.cpp)
package_add_test(world_stats ${WORLD_SOURCE_DIR}/stats.cpp)
package_add_test(world_contains ${WORLD_SOURCE_DIR}/contains.cpp)
package_add_test(world_capacity ${WORLD_SOURCE_DIR}/capacity.cpp)
package_add_test(world_view ${WORLD_SOURCE_DIR}/view.cpp)

set(VIEW_SOURCE_DIR ${TEST_SOURCE_DIR}/view)
package_add_test(view_capacity ${VIEW_SOURCE_DIR}/capacity.cpp)
package_add_test(view_get ${VIEW_SOURCE_DIR}/get.cpp)
package_add_test(view_iterate ${VIEW_SOURCE_DIR}/iterate.cpp)
package_add_test(view_lookup ${VIEW_SOURCE_DIR}/lookup.cpp)
