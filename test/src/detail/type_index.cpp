#include <gtest/gtest.h>
#include <ecfw/detail/type_index.hpp>

using namespace ecfw::detail;

TEST(type_index, equal) 
{
    ASSERT_EQ(type_index::type_id<int>(), type_index::type_id<const int>());
    ASSERT_EQ(type_index::type_id<int>(), type_index::type_id<int&>());
    ASSERT_EQ(type_index::type_id<int>(), type_index::type_id<const int&>());
    ASSERT_EQ(type_index::type_id<int>(), type_index::type_id<int&&>());
}
