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

	vector.construct(4096, ptr);
	ASSERT_EQ(ptr.use_count(), 2);

	vector.construct(768, ptr);
	ASSERT_EQ(ptr.use_count(), 3);

	vector.construct(8192, ptr);
	ASSERT_EQ(ptr.use_count(), 4);

	vector.construct(12000, ptr);
	ASSERT_EQ(ptr.use_count(), 5);

	vector.construct(16384, ptr);
	ASSERT_EQ(ptr.use_count(), 6);

	//////////////////////////////

	vector.destroy(12000);
	ASSERT_EQ(ptr.use_count(), 5);

	vector.destroy(768);
	ASSERT_EQ(ptr.use_count(), 4);

	vector.destroy(8192);
	ASSERT_EQ(ptr.use_count(), 3);

	vector.destroy(4096);
	ASSERT_EQ(ptr.use_count(), 2);

	vector.destroy(16384);
	ASSERT_EQ(ptr.use_count(), 1);
}

TEST(ComponentVectorTests, ObjectAccess) {
	std::shared_ptr<int> ptr = std::make_shared<int>(10);
	vector_t vector{};

	vector.construct(4096, ptr);
	ASSERT_EQ(ptr.use_count(), 2);

	vector.construct(768, ptr);
	ASSERT_EQ(ptr.use_count(), 3);

	vector.construct(8192, ptr);
	ASSERT_EQ(ptr.use_count(), 4);

	vector.construct(12000, ptr);
	ASSERT_EQ(ptr.use_count(), 5);

	vector.construct(16384, ptr);
	ASSERT_EQ(ptr.use_count(), 6);

	//////////////////////////////

	ASSERT_EQ(ptr, vector.at(8192).ptr);
	ASSERT_EQ(ptr, vector.at(768).ptr);
	ASSERT_EQ(ptr, vector.at(16384).ptr);
	ASSERT_EQ(ptr, vector.at(4096).ptr);
	ASSERT_EQ(ptr, vector.at(12000).ptr);
}
