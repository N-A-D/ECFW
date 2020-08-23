#include <random>
#include <gtest/gtest.h>
#include <ecfw/detail/dword.hpp>

TEST(integers, packing_and_unpacking)
{
	namespace dtl = ecfw::detail;

	std::mt19937 gen(std::random_device{}());
	std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
	uint32_t msw = dist(gen);
	uint32_t lsw = dist(gen);
	uint64_t word = dtl::concat(msw, lsw);
	ASSERT_EQ(msw, dtl::msw(word));
	ASSERT_EQ(lsw, dtl::lsw(word));
}