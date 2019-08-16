#include <gtest/gtest.h>
#include <ecfw/entity/traits.hpp>

using namespace ecfw;

TEST(EntityTraitTests, _64bitEntityTypeTests) {

	using entity_type = uint64_t;
	using traits_type = entity_traits<entity_type>;

	entity_type e0 = traits_type::create(0, 1);
	ASSERT_EQ(0, traits_type::index(e0));
	ASSERT_EQ(1, traits_type::version(e0));
	e0 = traits_type::recycle(e0);
	ASSERT_EQ(0, traits_type::index(e0));
	ASSERT_EQ(2, traits_type::version(e0));

	ASSERT_EQ(0xFFFFFFFF, traits_type::max_index);
	ASSERT_EQ(0xFFFFFFFF, traits_type::max_version);

}

TEST(EntityTraitTests, _32bitEntityTypeTests) {

	using entity_type = uint32_t;
	using traits_type = entity_traits<entity_type>;

	entity_type e0 = traits_type::create(0, 1);
	ASSERT_EQ(0, traits_type::index(e0));
	ASSERT_EQ(1, traits_type::version(e0));
	e0 = traits_type::recycle(e0);
	ASSERT_EQ(0, traits_type::index(e0));
	ASSERT_EQ(2, traits_type::version(e0));

	ASSERT_EQ(0xFFFFF, traits_type::max_index);
	ASSERT_EQ(0xFFF, traits_type::max_version);

}

TEST(EntityTraitTests, _16bitEntityTypeTests) {

	using entity_type = uint16_t;
	using traits_type = entity_traits<entity_type>;

	entity_type e0 = traits_type::create(0, 1);
	ASSERT_EQ(0, traits_type::index(e0));
	ASSERT_EQ(1, traits_type::version(e0));
	e0 = traits_type::recycle(e0);
	ASSERT_EQ(0, traits_type::index(e0));
	ASSERT_EQ(2, traits_type::version(e0));

	ASSERT_EQ(0xFFF, traits_type::max_index);
	ASSERT_EQ(0xF, traits_type::max_version);

}
