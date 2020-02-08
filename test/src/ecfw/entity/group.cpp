#include <gtest/gtest.h>
#include <ecfw/entity/group.hpp>
#include <algorithm>

using group_t = ecfw::detail::group;

TEST(GroupTests, Insertion) {
	group_t group;

	std::initializer_list<ecfw::u32> items{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	for (auto item : items) {
		group.insert(item);
	}
	
	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));

	// try reinserting items to elicit duplicates
	for (auto item : items) {
		group.insert(item);
	}

	// Check that no duplicates exist

	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));
}

TEST(GroupTests, Erasure) {
	group_t group;

	std::initializer_list<ecfw::u32> items{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	for (auto item : items) {
		group.insert(item);
	}

	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));

	items = {1, 2, 10, 4, 5, 6, 7, 8, 9};

	group.erase(3);

	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));

	items = { 9, 2, 10, 4, 8, 6, 7 };

	group.erase(1);
	group.erase(5);

	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));
}

TEST(GroupTests, Contains) {
	group_t group;

	std::initializer_list<ecfw::u32> items{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	for (auto item : items)
		group.insert(item);
	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));

	items = { 1, 2, 10, 4, 5, 6, 7, 8, 9 };
	group.erase(3);
	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));
	for (auto item : items)
		ASSERT_TRUE(group.contains(item));
	ASSERT_FALSE(group.contains(3));

	items = { 9, 2, 10, 4, 8, 6, 7 };
	group.erase(1);
	group.erase(5);
	ASSERT_TRUE(std::equal(items.begin(), items.end(), group.begin(), group.end()));
	for (auto item : items)
		ASSERT_TRUE(group.contains(item));
	ASSERT_FALSE(group.contains(5));
	ASSERT_FALSE(group.contains(1));
	ASSERT_FALSE(group.contains(3));
}
