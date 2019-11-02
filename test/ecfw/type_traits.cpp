#include <gtest/gtest.h>
#include <ecfw/meta/type_traits.hpp>
#include <vector>
#include <ecfw/component/storage.hpp>

using namespace ecfw::meta;

struct has {
	using type = void;
};

struct has_not {};

TEST(TypeTraitsTests, TraitTests) {

	static_assert(is_iterator_v<typename std::vector<int>::iterator>);
	static_assert(!is_iterator_v<int>);

	static_assert(has_type_member_v<has>);
	static_assert(!has_type_member_v<has_not>);

}