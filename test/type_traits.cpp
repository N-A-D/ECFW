#include <gtest/gtest.h>
#include <ecfw/meta/type_traits.hpp>
#include <vector>

using namespace ecfw::meta;

TEST(TypeTraitsTests, TraitTests) {

	static_assert(is_iterator_v<typename std::vector<int>::iterator>);
	static_assert(!is_iterator_v<int>);

}