#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/utility/traits.hpp>
#include <ecfw/utility/sparse_set.hpp>

using namespace ecfw::detail;

TEST(SparseSetTests, Interface) {
	ASSERT_TRUE(std::is_default_constructible_v<sparse_set>);
	ASSERT_TRUE(std::is_copy_constructible_v<sparse_set>);
	ASSERT_TRUE(std::is_copy_assignable_v<sparse_set>);
	ASSERT_TRUE(std::is_move_constructible_v<sparse_set>);
	ASSERT_TRUE(std::is_move_assignable_v<sparse_set>);

	sparse_set set;

	ASSERT_TRUE(set.empty());

	std::vector<ecfw::u32> list = { 1, 2, 3, 4, 5 };
	for (auto item : list)
		set.insert(item);

	ASSERT_FALSE(set.empty());
	ASSERT_EQ(set.size(), list.size());
	auto data = set.data();
	for (std::size_t i = 0; i != set.size(); ++i)
		ASSERT_EQ(*(data + i), list[i]);
	for (auto item : list)
		ASSERT_TRUE(set.contains(item));

	list = { 1, 5, 3, 4 };
	set.erase(2);
	ASSERT_FALSE(set.empty());
	ASSERT_EQ(set.size(), list.size());
	data = set.data();
	for (std::size_t i = 0; i != set.size(); ++i)
		ASSERT_EQ(*(data + i), list[i]);
	for (auto item : list)
		ASSERT_TRUE(set.contains(item));

	sparse_set copy{ set };
	ASSERT_FALSE(copy.empty());
	ASSERT_FALSE(set.empty());
	ASSERT_EQ(copy.size(), set.size());
	for (auto item : list)
		ASSERT_TRUE(copy.contains(item));

	sparse_set thief{ std::move(set) };
	ASSERT_FALSE(thief.empty());
	ASSERT_TRUE(set.empty());
	ASSERT_EQ(thief.size(), list.size());
	for (auto item : list)
		ASSERT_TRUE(thief.contains(item));

	thief.clear();
	ASSERT_TRUE(thief.empty());
	ASSERT_EQ(thief.size(), 0);
}

TEST(SparseSetTests, Iterator) {
	ASSERT_TRUE(is_iterator_v<sparse_set::iterator>);
	
	sparse_set set;

	std::initializer_list<ecfw::u32> list = { 1, 2, 3, 4, 5, 6, 7, 8 };
	for (auto item : list)
		set.insert(item);

	auto it = set.begin();

	ASSERT_EQ(*it, 1);
	ASSERT_EQ(*++it, 2);
	ASSERT_EQ(*it++, 2);
	ASSERT_EQ(*it, 3);
	ASSERT_EQ(*--it, 2);
	ASSERT_EQ(*it--, 2);
	ASSERT_EQ(*it, 1);

	auto other = set.begin();

	ASSERT_TRUE(other == it);
	++other;
	ASSERT_FALSE(other == it);

	ASSERT_TRUE(std::equal(list.begin(), list.end(), set.begin(), set.end()));
	std::vector<ecfw::u32> vector(list);
	ASSERT_TRUE(std::equal(vector.rbegin(), vector.rend(), set.rbegin(), set.rend()));
}
