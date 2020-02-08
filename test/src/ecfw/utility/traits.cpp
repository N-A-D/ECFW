#include <gtest/gtest.h>
#include <ecfw/utility/traits.hpp>
#include <vector>

using namespace ecfw::detail;

TEST(UtilityTraitsTests, IsIterator) {
	ASSERT_TRUE(is_iterator_v<std::vector<int>::iterator>);
	ASSERT_FALSE(is_iterator_v<char>);
}
