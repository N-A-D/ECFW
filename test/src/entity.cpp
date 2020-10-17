#include <random>
#include <gtest/gtest.h>
#include <ecfw/detail/entity.hpp>

TEST(integers, packing_and_unpacking)
{
	namespace dtl = ecfw::detail;

	std::mt19937 gen(std::random_device{}());
	std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
	uint32_t version = dist(gen);
	uint32_t index = dist(gen);
	uint64_t entity = dtl::make_entity(version, index);
	ASSERT_EQ(version, dtl::version(entity));
	ASSERT_EQ(index, dtl::index(entity));
}