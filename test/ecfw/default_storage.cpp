#include <gtest/gtest.h>
#include <ecfw/component/storage.hpp>

struct Boolean {
	Boolean(bool bit = false)
		: bit(bit) {}
	operator bool() const { return bit; }
	bool bit;
};

TEST(DefaultStorageTests, CreationTests) {
	ecfw::default_storage<Boolean> storage;
	ASSERT_TRUE(storage.construct(10, true));
	ASSERT_EQ(storage.size(), 11);
	ASSERT_TRUE(storage.construct(5, true));
}

TEST(DefaultStorageTests, RetrievalTests) {
	ecfw::default_storage<Boolean> storage;
	ASSERT_TRUE(storage.construct(10, true));
	ASSERT_EQ(storage.size(), 11);
	ASSERT_TRUE(storage.construct(5, true));
	ASSERT_FALSE(storage.get(0));
	ASSERT_FALSE(storage.get(1));
	ASSERT_FALSE(storage.get(2));
	ASSERT_TRUE(storage.get(5));
	ASSERT_TRUE(storage.get(10));
}