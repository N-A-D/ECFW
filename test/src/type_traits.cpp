#include <gtest/gtest.h>
#include <ecfw/detail/type_traits.hpp>

namespace dtl = ecfw::detail;

TEST(type_traits, is_iterator)
{
	struct iterator {};

	ASSERT_TRUE(dtl::is_iterator_v<std::vector<int>::iterator>);
	ASSERT_FALSE(dtl::is_iterator_v<iterator>);
}