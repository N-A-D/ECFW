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
	ASSERT_FALSE(set.contains(2));
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
	ASSERT_EQ(*(it += 2), 3);
	ASSERT_EQ(*(it -= 2), 1);
	ASSERT_EQ(*(set.begin() + 2), 3);
	ASSERT_EQ(*(set.end() - 2), 7);
	
	ASSERT_LT(it, set.end());
	ASSERT_LT(it, it + 2);
	ASSERT_LE(it, set.begin());
	ASSERT_LE(it, set.begin() + 3);
	ASSERT_GT(set.end(), it);
	ASSERT_GT(it + 2, it);
	ASSERT_GE(it, set.begin());
	ASSERT_GE(set.begin() + 4, it);

	ASSERT_EQ(set.begin(), set.begin());
	ASSERT_EQ(set.end(), set.end());
	ASSERT_EQ(sparse_set::iterator{}, sparse_set::iterator{});
	ASSERT_NE(set.begin() + 1, set.begin());
	
	ASSERT_TRUE(std::equal(std::begin(list),  std::end(list), std::begin(set), std::end(set)));
	ASSERT_TRUE(std::equal(std::rbegin(list), std::rend(list) , std::rbegin(set), std::rend(set)));

}
