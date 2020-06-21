#include <set>
#include <vector>
#include <random>
#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/util/sparse_set.hpp>

using namespace ecfw;

TEST(SparseSetTests, Interface) {
	// Default construction
	sparse_set s{};
	ASSERT_TRUE(s.empty());
	ASSERT_EQ(s.size(), 0);

	std::vector<ecfw::u64> data{ 15000, 30000, 45000, 60000, 75000, 90000, 105000 };
	std::vector<ecfw::u64> nonexistent_data{ 14999, 30001, 44999, 60001, 74999, 90001, 105001 };

	std::mt19937 gen{ std::random_device{}() };
	std::shuffle(data.begin(), data.end(), gen);

	// Test sparse_set::insert
	// Insert unique items
	for (auto item : data)
		s.insert(item);
	ASSERT_FALSE(s.empty());
	ASSERT_EQ(s.size(), data.size());

	// Try to reinsert existing items
	for (auto item : data)
		s.insert(item);
	ASSERT_FALSE(s.empty());
	ASSERT_EQ(s.size(), data.size());

	// Test sparse_set::contains
	// Check if the set contains the original data
	std::shuffle(data.begin(), data.end(), gen);
	for (auto item : data)
		ASSERT_TRUE(s.contains(item));

	// Check if the set contains non existent data
	for (auto item : nonexistent_data)
		ASSERT_FALSE(s.contains(item));

	// Test sparse_set::data
	auto raw_storage = s.data();
	std::ptrdiff_t length = static_cast<std::ptrdiff_t>(s.size());
	std::set<ecfw::u64> std_set{ data.begin(), data.end() };
	// Check that what's in the raw_storage can be found in the original data set
	for (auto i = 0; i != length; ++i)
		ASSERT_TRUE(std_set.find(raw_storage[i]) != std_set.end());
	// Check that each item in the original data set is in the set's storage
	for (auto i = std_set.begin(); i != std_set.end(); ++i) {
		ASSERT_NE(std::find(raw_storage, raw_storage + length, *i), raw_storage + length);
	}

	// Test sparse_set::erase
	// Try to erase nonexistent data
	for (auto item : nonexistent_data)
		s.erase(item);
	ASSERT_FALSE(s.empty());
	ASSERT_EQ(s.size(), data.size());

	// Erase existing data. Ensure that all 'erased' data remains erased and that
	// all data not erased yet remains locatable in the set
	std::shuffle(data.begin(), data.end(), gen);
	for (std::ptrdiff_t i = 0; i < length; ++i) {
		s.erase(data[i]);
		for (std::ptrdiff_t j = i; j >= 0; --j)
			ASSERT_FALSE(s.contains(data[j]));
		for (std::ptrdiff_t j = i + 1; j < length; ++j)
			ASSERT_TRUE(s.contains(data[j]));
		ASSERT_EQ(s.size(), data.size() - i - 1);
	}
	ASSERT_TRUE(s.empty());

	// Test sparse_set::clear
	for (auto item : data)
		s.insert(item);
	ASSERT_TRUE(!s.empty());
	ASSERT_EQ(s.size(), data.size());
	s.clear();
	ASSERT_TRUE(s.empty());
	ASSERT_EQ(s.size(), 0);
	for (auto item : data)
		ASSERT_FALSE(s.contains(item));

	std::shuffle(data.begin(), data.end(), gen);
	for (auto item : data)
		s.insert(item);
	ASSERT_FALSE(s.empty());
	ASSERT_EQ(s.size(), data.size());

	// Move construction
	sparse_set theif_by_construction{ std::move(s) };
	ASSERT_FALSE(theif_by_construction.empty());
	ASSERT_EQ(theif_by_construction.size(), data.size());
	ASSERT_TRUE(s.empty());
	ASSERT_EQ(s.size(), 0);

	for (auto item : data)
		ASSERT_TRUE(theif_by_construction.contains(item));

	// Move assignment
	s = std::move(theif_by_construction);
	ASSERT_TRUE(theif_by_construction.empty());
	ASSERT_EQ(theif_by_construction.size(), 0);
	ASSERT_FALSE(s.empty());
	ASSERT_EQ(s.size(), data.size());
}

TEST(SparseSetTests, Iterator) {
	using iterator_t = sparse_set::iterator;
	using vi_t = std::vector<ecfw::u64>::iterator;

	sparse_set s{};
	std::vector<ecfw::u64> data{ 60000, 75000, 15000, 30000, 90000, 105000, 45000 };
	for (auto item : data)
		s.insert(item);

	ASSERT_TRUE(std::equal(data.begin(), data.end(), s.begin(), s.end()));
	ASSERT_TRUE(std::equal(data.rbegin(), data.rend(), s.rbegin(), s.rend()));

	ASSERT_EQ(data.end() - data.begin(), s.end() - s.begin());

	iterator_t it = s.begin();
	vi_t invariant = data.begin();

	ASSERT_EQ(*it, *invariant);
	ASSERT_EQ(*it++, *invariant++);
	ASSERT_EQ(*it, *invariant);
	ASSERT_EQ(*++it, *++invariant);
	ASSERT_EQ(*(it += 2), *(invariant += 2));
	ASSERT_EQ(*(it + 2), *(it + 2));
	ASSERT_EQ(*it--, *invariant--);
	ASSERT_EQ(*it, *invariant);
	ASSERT_EQ(*--it, *--invariant);
	ASSERT_EQ(*(it -= 1), *(invariant -= 1));
	ASSERT_EQ(*(it - 1), *(invariant - 1));

	ASSERT_LT(s.begin(), s.end());
	ASSERT_LE(s.begin(), s.begin());
	ASSERT_GT(s.begin() + 1, s.begin());
	ASSERT_GE(s.begin(), s.begin());
	ASSERT_EQ(s.begin(), s.begin());
	ASSERT_EQ(s.end(), s.end());
	ASSERT_NE(s.begin(), s.end());

	for (auto i = 0; i < data.size() - 1; ++i) {
		ASSERT_LT(s.begin() + i, s.end());
		ASSERT_GT(s.end(), s.begin() + i);
	}

	for (auto i = 0; i < data.size(); ++i) {
		ASSERT_LE(s.begin() + i, s.end());
		ASSERT_GE(s.end(), s.begin() + i);
	}

	for (std::ptrdiff_t i = data.size() - 1; i > 0; --i) {
		ASSERT_GT(s.end() - i, s.begin());
		ASSERT_LT(s.begin(), s.end() - i);
	}

	for (std::ptrdiff_t i = data.size() - 1; i >= 0; --i) {
		ASSERT_GE(s.end() - i, s.begin());
		ASSERT_LE(s.begin(), s.end() - i);
	}
}
