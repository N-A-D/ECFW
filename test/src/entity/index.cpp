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

TEST(EntityIndexTests, BitmaskGenerationTests) {
	Assert a{};
	ASSERT_TRUE(a.output());
	constexpr auto component_types = typelist<Comp<0>, Comp<1>, Comp<3>>{};
	constexpr bool output = (bitmask<4>{11} == encode_subset_as_bitmask(component_types, all_component_types));
	static_assert(output);
	ASSERT_TRUE(output);
}

TEST(EntityIndexTests, SparseSetFunctionalityTests) {
	std::vector<ecfw::u32> integers_with_duplicates = { 1, 2, 2, 3, 4, 5, 5, 5, 6, 6 };
	std::vector<ecfw::u32> integers_without_duplicates = { 1, 2, 3, 4, 5, 6 };
	sparse_set s0{};
	ASSERT_TRUE(s0.empty());
	for (auto integer : integers_with_duplicates) s0.insert(integer);

	ASSERT_TRUE(std::equal(integers_without_duplicates.begin(), integers_without_duplicates.end(), s0.begin(), s0.end()));

	sparse_set s1{ s0 };
	ASSERT_EQ(s1.size(), s0.size());

	sparse_set s2{ std::move(s1) };
	ASSERT_TRUE(s1.empty());
	ASSERT_EQ(s2.size(), s0.size());

	auto begin = s2.data();
	auto end = s2.data() + s2.size();

	ASSERT_TRUE(std::equal(s0.begin(), s0.end(), begin, end));

	for (auto integer : integers_with_duplicates) ASSERT_TRUE(s0.contains(integer));

	auto integers = { 1, 6, 3, 4, 5 };
	s0.erase(2);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s0.begin(), s0.end()));

	integers = { 1, 6, 3, 4 };
	s0.erase(5);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s0.begin(), s0.end()));

	integers = { 4, 6, 3 };
	s0.erase(1);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s0.begin(), s0.end()));

	integers = { 4, 3 };
	s0.erase(6);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s0.begin(), s0.end()));

	integers = { 4 };
	s0.erase(3);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s0.begin(), s0.end()));
}

TEST(EntityIndexTests, SparseSetIteratorTests) {
	sparse_set set{};
	std::initializer_list<ecfw::u32> integers = { 1, 2, 3, 4, 5, 6 };
	for (auto integer : integers) set.insert(integer);

	auto it = set.begin();
	ASSERT_EQ(*it++, 1);
	ASSERT_EQ(*it--, 2);
	ASSERT_EQ(*++it, 2);
	ASSERT_EQ(*--it, 1);
	ASSERT_EQ(*(it + 4), 5);
	ASSERT_EQ(*(it += 5), 6);
	ASSERT_EQ(*(it - 5), 1);
	ASSERT_EQ(*(it -= 5), 1);
	ASSERT_EQ(set.end() - set.begin(), set.size());
	ASSERT_EQ((it + 2) - set.begin(), 2);
	ASSERT_LT(set.begin(), set.end());
	ASSERT_LT(set.begin(), set.begin() + 1);
	ASSERT_LE(set.begin(), set.begin());
	ASSERT_LE(set.begin(), set.end());
	ASSERT_GT(set.end(), set.begin());
	ASSERT_GT(set.end(), set.end() - 2);
	ASSERT_GE(set.end(), set.end());
	ASSERT_GE(set.end(), set.end() - 2);
	ASSERT_GE(set.begin() + 1, set.begin());
	ASSERT_EQ(set.begin() + set.size(), set.end());
	ASSERT_EQ(set.begin(), set.begin());
	ASSERT_EQ(set.end(), set.end());
	ASSERT_NE(set.begin(), set.end());
	ASSERT_NE(set.begin() + 1, set.begin() + 2);
}