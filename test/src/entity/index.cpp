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
	std::vector<ecfw::u64> integers_with_duplicates = { 10, 200, 200, 3000, 40000, 50000, 50000, 50000, 600000, 600000 };
	std::vector<ecfw::u64> integers_without_duplicates = { 10, 200, 3000, 40000, 50000, 600000 };
	sparse_set s0{};
	ASSERT_TRUE(s0.empty());
	for (auto integer : integers_with_duplicates) s0.insert(integer);

	ASSERT_TRUE(std::equal(integers_without_duplicates.begin(), integers_without_duplicates.end(), s0.begin(), s0.end()));

	sparse_set s1{ std::move(s0) };

	ASSERT_EQ(s1.size(), integers_without_duplicates.size());
	ASSERT_TRUE(s0.empty());

	ASSERT_TRUE(std::equal(s1.begin(), s1.end(), integers_without_duplicates.begin(), integers_without_duplicates.end()));
	
	for (auto integer : integers_without_duplicates)
		ASSERT_TRUE(s1.contains(integer));

	std::vector<ecfw::u64> integers = { 10, 600000, 3000, 40000, 50000 };
	s1.erase(200);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s1.begin(), s1.end()));

	integers = { 10, 600000, 3000, 40000 };
	s1.erase(50000);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s1.begin(), s1.end()));

	integers = { 40000, 600000, 3000 };
	s1.erase(10);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s1.begin(), s1.end()));

	integers = { 40000, 3000 };
	s1.erase(600000);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s1.begin(), s1.end()));

	integers = { 40000 };
	s1.erase(3000);
	ASSERT_TRUE(std::equal(integers.begin(), integers.end(), s1.begin(), s1.end()));
}

TEST(EntityIndexTests, SparseSetIteratorTests) {
	sparse_set set{};
	std::initializer_list<ecfw::u64> integers = { 1, 2, 3, 4, 5, 6 };
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