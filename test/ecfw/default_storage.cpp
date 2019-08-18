#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include <ecfw/component/storage.hpp>

struct Boolean {
	Boolean(bool bit = false)
		: bit(bit) {}
	operator bool() const { return bit; }
	bool bit;
};

constexpr size_t N = 100;

TEST(DefaultStorageTests, CreationTests) {
	ecfw::default_storage<Boolean> storage;
	for (size_t i = 0; i != N; ++i) {
		ASSERT_TRUE(storage.construct(i, true));
	}
	storage.clear();
	ASSERT_TRUE(storage.empty());
	std::vector<size_t> indices(N);
	std::generate(indices.begin(), indices.end(), [n = 0]() mutable { return n++; });
	std::shuffle(indices.begin(), indices.end(), std::mt19937{ std::random_device{}() });
	for (auto index : indices) {
		ASSERT_TRUE(storage.construct(index, true));
	}
}

TEST(DefaultStorageTests, RetrievalTests) {
	ecfw::default_storage<Boolean> storage;
	for (size_t i = 0; i != N; ++i) {
		ASSERT_TRUE(storage.construct(i, true));
	}
	for (size_t i = 0; i != N; ++i) {
		ASSERT_TRUE(storage.get(i));
	}
	storage.clear();
	ASSERT_TRUE(storage.empty());
	std::vector<size_t> indices(N);
	std::generate(indices.begin(), indices.end(), [n = 0]() mutable { return n++; });
	std::shuffle(indices.begin(), indices.end(), std::mt19937{ std::random_device{}() });
	for (auto index : indices) {
		ASSERT_TRUE(storage.construct(index, true));
	}
	std::sort(indices.begin(), indices.end());
	for (auto index : indices) {
		ASSERT_TRUE(storage.get(index));
	}
}
