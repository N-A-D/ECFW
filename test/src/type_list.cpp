#include <gtest/gtest.h>
#include <ecfw/detail/type_list.hpp>

namespace dtl = ecfw::detail;
namespace bh = boost::hana;

TEST(type_list, is_unique)
{
    constexpr auto unique_list = dtl::type_list_v<int, char, bool>;
    constexpr auto list = dtl::type_list_v<int, char, bool, int, bool>;

    static_assert(dtl::is_unique(unique_list), "");
    static_assert(!dtl::is_unique(list));

    ASSERT_TRUE(dtl::is_unique(unique_list));
    ASSERT_FALSE(dtl::is_unique(list));
}

TEST(type_list, index_of)
{
    constexpr auto list = dtl::type_list_v<int, char, bool>;
    constexpr auto int_type = dtl::type_v<int>;
    constexpr auto char_type = dtl::type_v<char>;
    constexpr auto bool_type = dtl::type_v<bool>;

    static_assert(dtl::index_of(int_type, list) == bh::size_c<0>);
    static_assert(dtl::index_of(char_type, list) == bh::size_c<1>);
    static_assert(dtl::index_of(bool_type, list) == bh::size_c<2>);

    ASSERT_EQ(dtl::index_of(int_type, list), bh::size_c<0>);
    ASSERT_EQ(dtl::index_of(char_type, list), bh::size_c<1>);
    ASSERT_EQ(dtl::index_of(bool_type, list), bh::size_c<2>);
}