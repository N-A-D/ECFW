#include <gtest/gtest.h>
#include <ecfw/meta/type_traits.hpp>
#include <vector>
#include <ecfw/component/storage.hpp>

using namespace ecfw::meta;

struct TestComponent0 {};

struct TestComponent1 {};

template <class T>
struct CustomStorage {};

namespace ecfw {
	template <> 
	struct underlying_storage<TestComponent0> {}; // Does not define ::type

	template <>
	struct underlying_storage<TestComponent1> {
		using type = CustomStorage<TestComponent1>;
	};
}

TEST(TypeTraitsTests, TraitTests) {

	static_assert(is_iterator_v<typename std::vector<int>::iterator>);
	static_assert(!is_iterator_v<int>);

	static_assert(!underlying_storage_has_type_member_v<TestComponent0>);
	static_assert(underlying_storage_has_type_member_v<TestComponent1>);

}