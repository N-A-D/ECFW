#include <gtest/gtest.h>
#include <ecfw/component/vector.hpp>
#include <memory>

struct object {
	object(std::shared_ptr<int> ptr)
		: ptr(ptr)
	{}
	std::shared_ptr<int> ptr{};
};

using vector_t = ecfw::detail::vector<object>;

TEST(ComponentVectorTests, ObjectConstructionAndDestruction) {
	std::shared_ptr<int> ptr = std::make_shared<int>(10);
	vector_t vector{};

	// Test construction

	vector.construct(4095, ptr);
	ASSERT_EQ(ptr.use_count(), 2);

	vector.construct(767, ptr);
	ASSERT_EQ(ptr.use_count(), 3);

	vector.construct(8191, ptr);
	ASSERT_EQ(ptr.use_count(), 4);

	vector.construct(12000, ptr);
	ASSERT_EQ(ptr.use_count(), 5);

	vector.construct(16383, ptr);
	ASSERT_EQ(ptr.use_count(), 6);

	ASSERT_EQ(vector.size(), 16384);

	// Test destruction

	vector.destroy(12000);
	ASSERT_EQ(ptr.use_count(), 5);

	vector.destroy(767);
	ASSERT_EQ(ptr.use_count(), 4);

	vector.destroy(8191);
	ASSERT_EQ(ptr.use_count(), 3);

	vector.destroy(4095);
	ASSERT_EQ(ptr.use_count(), 2);

	vector.destroy(16383);
	ASSERT_EQ(ptr.use_count(), 1);
}

TEST(ComponentVectorTests, ObjectAccess) {
	std::shared_ptr<int> ptr = std::make_shared<int>(10);
	vector_t vector{};

	// Test construction
	vector.construct(4095, ptr);
	ASSERT_EQ(ptr.use_count(), 2);

	vector.construct(767, ptr);
	ASSERT_EQ(ptr.use_count(), 3);

	vector.construct(8191, ptr);
	ASSERT_EQ(ptr.use_count(), 4);

	vector.construct(12000, ptr);
	ASSERT_EQ(ptr.use_count(), 5);

	vector.construct(16383, ptr);
	ASSERT_EQ(ptr.use_count(), 6);

	ASSERT_EQ(vector.size(), 16384);

	// Test the at function

	ASSERT_EQ(ptr, vector.at(8191).ptr);
	ASSERT_EQ(ptr, vector.at(767).ptr);
	ASSERT_EQ(ptr, vector.at(16383).ptr);
	ASSERT_EQ(ptr, vector.at(4095).ptr);
	ASSERT_EQ(ptr, vector.at(12000).ptr);

	// Test destruction

	vector.destroy(12000);
	ASSERT_EQ(ptr.use_count(), 5);

	vector.destroy(767);
	ASSERT_EQ(ptr.use_count(), 4);

	vector.destroy(8191);
	ASSERT_EQ(ptr.use_count(), 3);

	vector.destroy(4095);
	ASSERT_EQ(ptr.use_count(), 2);

	vector.destroy(16383);
	ASSERT_EQ(ptr.use_count(), 1);
}
