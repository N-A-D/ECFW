#include <gtest/gtest.h>
#include <ecfw/entity/entity.hpp>

TEST(EntityTests, PackingAndUnpacking) {
	const ecfw::u32 limit = 4096;
	for (auto v = 1; v != limit; ++v) {
		for (auto i = 1; i != limit; ++i) {
			auto entity = ecfw::detail::make_entity(v, i);
			auto [v_, i_] = ecfw::detail::unpack_entity(entity);
			ASSERT_EQ(v, v_);
			ASSERT_EQ(i, i_);
		}
	}
}