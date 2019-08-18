#include <gtest/gtest.h>
#include <random>
#include <algorithm>
#include <ecfw/entity/group.hpp>

using entity_type = uint16_t;
using group_type = ecfw::entity_group<entity_type>;

constexpr size_t N = 100;

std::mt19937 gen{ std::random_device{}() };

TEST(EntityGroupTests, MemberInsertionTests) {
	group_type group;
	for (entity_type i = 0; i != N; ++i) {
		group.insert(i);
	}
	for (entity_type i = 0; i != N; ++i) {
		ASSERT_TRUE(group.contains(i));
	}

	group_type group2;
	std::vector<entity_type> entities(N);
	std::generate(entities.begin(), entities.end(), [n = 0]() mutable { return n++; });
	std::shuffle(entities.begin(), entities.end(), gen);
	for (auto entity : entities) {
		group2.insert(entity);
	}
	for (auto entity : entities) {
		ASSERT_TRUE(group2.contains(entity));
	}
}

TEST(EntityGroupTests, MemberRemovalTests) {
	group_type group;
	for (entity_type i = 0; i != N; ++i) {
		group.insert(i);
	}
	for (entity_type i = 0; i != N; ++i) {
		ASSERT_TRUE(group.contains(i));
	}
	for (entity_type i = 0; i != N; ++i) {
		group.erase(i);
	}
	for (entity_type i = 0; i != N; ++i) {
		ASSERT_FALSE(group.contains(i));
	}

	group_type group2;
	std::vector<entity_type> entities;
	std::generate(entities.begin(), entities.end(), [n = 0]() mutable { return n++;  });
	std::shuffle(entities.begin(), entities.end(), gen);
	for (auto entity : entities) {
		group2.insert(entity);
	}
	for (auto entity : entities) {
		ASSERT_TRUE(group2.contains(entity));
	}
	for (auto entity : entities) {
		group.erase(entity);
	}
	for (auto entity : entities) {
		ASSERT_FALSE(group2.contains(entity));
	}
}

TEST(EntityGroupTests, GroupClearingTests) {
	group_type group;
	for (entity_type i = 0; i != N; ++i) {
		group.insert(i);
	}
	for (entity_type i = 0; i != N; ++i) {
		ASSERT_TRUE(group.contains(i));
	}
	ASSERT_FALSE(group.empty());
	ASSERT_EQ(group.size(), N);
	group.clear();
	ASSERT_TRUE(group.empty());
	ASSERT_EQ(group.size(), 0);
}