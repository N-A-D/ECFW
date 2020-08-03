#include <random>
#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/detail/sparse_set.hpp>

using sparse_set = ecfw::detail::sparse_set;

TEST(sparse_set, nonexisting_element_insertion) {
	sparse_set ss{};

	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };

	std::mt19937 gen{ std::random_device{}() };
	std::shuffle(data.begin(), data.end(), gen);

	for (auto item : data)
		ss.insert(item);

	ASSERT_TRUE(std::all_of(data.begin(), data.end(), [&ss](auto item) { return ss.contains(item); }));
	ASSERT_TRUE(std::equal(data.begin(), data.end(), ss.begin(), ss.end()));
	ASSERT_EQ(ss.size(), data.size());
	ASSERT_FALSE(ss.empty());
	
}

TEST(sparse_set, existing_element_reinsertion) {
	sparse_set ss{};

	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };

	std::mt19937 gen{ std::random_device{}() };
	std::shuffle(data.begin(), data.end(), gen);

	for (auto item : data)
		ss.insert(item);

	ASSERT_TRUE(std::all_of(data.begin(), data.end(), [&ss](auto item) { return ss.contains(item); }));
	ASSERT_TRUE(std::equal(data.begin(), data.end(), ss.begin(), ss.end()));
	ASSERT_EQ(ss.size(), data.size());
	ASSERT_FALSE(ss.empty());

	for (auto item : data)
		ss.insert(item);

	ASSERT_TRUE(std::all_of(data.begin(), data.end(), [&ss](auto item) { return ss.contains(item); }));
	ASSERT_TRUE(std::equal(data.begin(), data.end(), ss.begin(), ss.end()));
	ASSERT_EQ(ss.size(), data.size());
	ASSERT_FALSE(ss.empty());
}

TEST(sparse_set, existing_element_removal) {
	sparse_set ss{};

	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };

	std::mt19937 gen{ std::random_device{}() };
	std::shuffle(data.begin(), data.end(), gen);

	for (auto item : data)
		ss.insert(item);

	ASSERT_TRUE(std::all_of(data.begin(), data.end(), [&ss](auto item) { return ss.contains(item); }));
	ASSERT_TRUE(std::equal(data.begin(), data.end(), ss.begin(), ss.end()));
	ASSERT_EQ(ss.size(), data.size());
	ASSERT_FALSE(ss.empty());

	for (auto item : data)
		ss.erase(item);

	ASSERT_EQ(ss.size(), 0);
	ASSERT_TRUE(ss.empty());
	ASSERT_FALSE(std::all_of(data.begin(), data.end(), [&ss](auto item) { return ss.contains(item); }));
}

TEST(sparse_set, nonexisting_element_removal) {
	sparse_set ss{};

	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };

	std::mt19937 gen{ std::random_device{}() };
	std::shuffle(data.begin(), data.end(), gen);

	for (auto item : data)
		ss.insert(item);

	ASSERT_TRUE(std::all_of(data.begin(), data.end(), [&ss](auto item) { return ss.contains(item); }));
	ASSERT_TRUE(std::equal(data.begin(), data.end(), ss.begin(), ss.end()));
	ASSERT_EQ(ss.size(), data.size());
	ASSERT_FALSE(ss.empty());

	std::vector<uint64_t> nonexistent_data{ 14999, 30001, 44999, 60001, 74999, 90001, 105001 };
	
	ASSERT_FALSE(std::all_of(nonexistent_data.begin(), nonexistent_data.end(), [&ss](auto item) { return ss.contains(item); }));

	for (auto item : nonexistent_data)
		ss.erase(item);

	ASSERT_TRUE(std::all_of(data.begin(), data.end(), [&ss](auto item) { return ss.contains(item); }));
	ASSERT_TRUE(std::equal(data.begin(), data.end(), ss.begin(), ss.end()));
	ASSERT_EQ(ss.size(), data.size());
	ASSERT_FALSE(ss.empty());

	for (auto item : nonexistent_data)
		ASSERT_FALSE(ss.contains(item));
	ASSERT_FALSE(std::all_of(nonexistent_data.begin(), nonexistent_data.end(), [&ss](auto item) { return ss.contains(item); }));
}

TEST(sparse_set, iterator_pre_increment) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);

	auto invariant_it = data.begin();
	auto candidate_it = ss.begin();

	++invariant_it;
	++candidate_it;

	ASSERT_EQ(*invariant_it, *candidate_it);
}

TEST(sparse_set, iterator_post_increment) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);

	auto invariant_it = data.begin();
	auto candidate_it = ss.begin();

	invariant_it++;
	candidate_it++;

	ASSERT_EQ(*invariant_it, *candidate_it);
}

TEST(sparse_set, iterator_pre_decrement) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);

	auto invariant_it = data.end();
	auto candidate_it = ss.end();

	--invariant_it;
	--candidate_it;

	ASSERT_EQ(*invariant_it, *candidate_it);
}

TEST(sparse_set, iterator_post_decrement) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);
	auto invariant_it = data.begin();
	auto candidate_it = ss.begin();

	++invariant_it;
	++candidate_it;
	ASSERT_EQ(*invariant_it, *candidate_it);

	invariant_it--;
	candidate_it--;

	ASSERT_EQ(*invariant_it, *candidate_it);
}

TEST(sparse_set, iterator_compound_addition_assignment) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);
	auto invariant_it = data.begin();
	auto candidate_it = ss.begin();

	invariant_it += 2;
	candidate_it += 2;

	ASSERT_EQ(*invariant_it, *candidate_it);
}

TEST(sparse_set, iterator_compound_subtraction_assignment) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);
	auto invariant_it = data.end();
	auto candidate_it = ss.end();

	invariant_it -= 2;
	candidate_it -= 2;

	ASSERT_EQ(*invariant_it, *candidate_it);
}

TEST(sparse_set, iterator_addition) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);
	auto invariant_it = data.begin();
	auto candidate_it = ss.begin();

	ASSERT_EQ(*(invariant_it + 2), *(candidate_it + 2));
}

TEST(sparse_set, iterator_subtraction) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);
	auto invariant_it = data.end();
	auto candidate_it = ss.end();

	ASSERT_EQ(*(invariant_it - 2), *(candidate_it - 2));
}

TEST(sparse_set, iterator_equals) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);

	ASSERT_EQ(ss.begin(), ss.begin());
	ASSERT_EQ(ss.end(), ss.end());
}

TEST(sparse_set, iterator_not_equals) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);

	ASSERT_NE(ss.begin(), ss.end());
	ASSERT_NE(ss.begin(), ss.begin() + 1);
}

TEST(sparse_set, iterator_less_than) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);
	ASSERT_LT(ss.begin(), ss.begin() + 1);
	ASSERT_LT(ss.begin(), ss.end());
}

TEST(sparse_set, iteraor_less_than_or_equal_to) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);
	ASSERT_LE(ss.begin(), ss.begin());
	ASSERT_LE(ss.begin(), ss.begin() + 1);
	ASSERT_LE(ss.begin(), ss.end());
}

TEST(sparse_set, iterator_greater_than) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);

	ASSERT_GT(ss.end(), ss.begin());
	ASSERT_GT(ss.end(), ss.end() - 1);
}

TEST(sparse_set, iterator_greater_than_or_equal_to) {
	sparse_set ss{};
	std::vector<uint64_t> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	for (auto item : data)
		ss.insert(item);

	ASSERT_GE(ss.end(), ss.end());
	ASSERT_GE(ss.end(), ss.begin());
	ASSERT_GE(ss.end(), ss.end() - 1);
}
