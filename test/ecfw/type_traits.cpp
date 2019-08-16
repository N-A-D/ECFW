#include <gtest/gtest.h>
#include <ecfw/meta/type_traits.hpp>
#include <vector>

using namespace ecfw::meta;

TEST(TypeTraitsTests, TraitTests) {

	static_assert(is_iterator_v<typename std::vector<int>::iterator>);
	static_assert(!is_iterator_v<int>);

	static_assert(std::is_same_v<int, type_identity_t<int>>);
	static_assert(!std::is_same_v<char, type_identity_t<int>>);

}