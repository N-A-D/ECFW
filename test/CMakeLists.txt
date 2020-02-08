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

#package_add_test(component_mask_tests "src/ecfw/component/mask.cpp")
package_add_test(component_trait_tests "src/ecfw/component/traits.cpp")
package_add_test(component_vector_tests "src/ecfw/component/vector.cpp")

package_add_test(entity_id_tests "src/ecfw/entity/entity.cpp")
package_add_test(entity_group_tests "src/ecfw/entity/group.cpp")

#package_add_test(hana_index_of_tests "src/ecfw/utility/index_of.cpp")
package_add_test(utility_traits_tests "src/ecfw/utility/traits.cpp")