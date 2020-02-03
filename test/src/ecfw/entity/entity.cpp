#include <gtest/gtest.h>
#include <ecfw/entity/entity.hpp>

using namespace ecfw;
using namespace detail;

TEST(EntityIdTests, AssemblyAndDisassemly) {
	for (u32 index = 1; index != 10; ++index) {
		for (u32 version = 1; version != 10; ++version) {
			u64 eid = make_eid(version, index);
			auto [v, i] = unpack_eid(eid);
			ASSERT_EQ(version, v);
			ASSERT_EQ(index, i);
		}
	}
}