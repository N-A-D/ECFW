#include <vector>
#include <gtest/gtest.h>
#include <ecfw/util/is_iterator.hpp>

using namespace ecfw;
using namespace ecfw::detail;

TEST(UtilityTests, IsIterator) {
	static_assert(is_iterator_v<std::vector<int>::iterator>);
	ASSERT_TRUE(is_iterator_v<std::vector<int>::iterator>);
	static_assert(!is_iterator_v<float>);
	ASSERT_TRUE(!is_iterator_v<float>);
}