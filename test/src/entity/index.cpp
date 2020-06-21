#include <gtest/gtest.h>
#include <ecfw/entity/index.hpp>

using namespace ecfw::detail;

template <typename... Ts>
using typelist = hana::tuple<hana::basic_type<Ts>...>;

template <std::size_t>
struct Comp {};

constexpr auto all_component_types = typelist<Comp<0>, Comp<1>, Comp<2>, Comp<3>>{};

struct Assert {
	bool output() const {
		constexpr bitmask<4> invariant{5};
		constexpr auto component_types = typelist<Comp<0>, Comp<2>>{};
		if constexpr (invariant == encode_subset_as_bitmask(component_types, all_component_types))
			return true;
		else
			return false;
	}
};

TEST(EntityIndexTests, BitmaskGeneration) {
	Assert a{};
	ASSERT_TRUE(a.output());
	constexpr auto component_types = typelist<Comp<0>, Comp<1>, Comp<3>>{};
	constexpr bool output = (bitmask<4>{11} == encode_subset_as_bitmask(component_types, all_component_types));
	static_assert(output);
	ASSERT_TRUE(output);
}
