#include <gtest/gtest.h>
#include <ecfw/detail/type_index.hpp>

using namespace ecfw::detail;

TEST(type_index, equal) 
{
    ASSERT_EQ(type_index<int>(), type_index<const int>());
    ASSERT_EQ(type_index<int>(), type_index<int&>());
    ASSERT_EQ(type_index<int>(), type_index<const int&>());
    ASSERT_EQ(type_index<int>(), type_index<int&&>());
}
