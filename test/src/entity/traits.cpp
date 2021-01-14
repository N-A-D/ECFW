#include <gtest/gtest.h>
#include <ecfw/entity.hpp>
#include <ecfw/world.hpp>

TEST(entity_traits_64b, index) {
    using traits_type = ecfw::entity_traits<std::uint64_t>;
    ASSERT_EQ(traits_type::index(0xFFFFFFFF0000000F), 0x0000000F);
}

TEST(entity_traits_64b, version) {
    using traits_type = ecfw::entity_traits<std::uint64_t>;
    ASSERT_EQ(traits_type::version(0x0000000FFFFFFFFF), 0xF);
}

TEST(entity_traits_64b, is_recyclable) {
    using traits_type = ecfw::entity_traits<std::uint64_t>;
    ASSERT_TRUE(traits_type::is_recyclable(0x0000000FFFFFFFFF));
    ASSERT_FALSE(traits_type::is_recyclable(0xFFFFFFFFF0000000));
}

TEST(entity_traits_64b, combine) {
    using traits_type = ecfw::entity_traits<std::uint64_t>;
    std::uint64_t e = traits_type::combine(0xFFFF0000, 0xF0000000);
    ASSERT_EQ(e, 0xF0000000FFFF0000);
}

TEST(entity_traits_32b, index) {
    using traits_type = ecfw::entity_traits<std::uint32_t>;
    ASSERT_EQ(traits_type::index(0xFFFF0000), 0xF0000);
}

TEST(entity_traits_32b, version) {
    using traits_type = ecfw::entity_traits<std::uint32_t>;
    ASSERT_EQ(traits_type::version(0xFFFF0000), 0xFFF);
}

TEST(entity_traits_32b, is_recyclable) {
    using traits_type = ecfw::entity_traits<std::uint32_t>;
    ASSERT_TRUE(traits_type::is_recyclable(0xF0000000));
    ASSERT_FALSE(traits_type::is_recyclable(0xFFF00000));
}

TEST(entity_traits_32b, combine) {
    using traits_type = ecfw::entity_traits<std::uint32_t>;
    std::uint32_t e = traits_type::combine(0x00FF, 0xF0F);
    ASSERT_EQ(e, 0xF0F000FF);
}

TEST(entity_traits_16b, index) {
    using traits_type = ecfw::entity_traits<std::uint16_t>;
    ASSERT_EQ(traits_type::index(0x0FFF), 0xFFF);
}

TEST(entity_traits_16b, version) {
    using traits_type = ecfw::entity_traits<std::uint16_t>;
    ASSERT_EQ(traits_type::version(0xE000), 0xE);
}

TEST(entity_traits_16b, is_recyclable) {
    using traits_type = ecfw::entity_traits<std::uint16_t>;
    ASSERT_TRUE(traits_type::is_recyclable(0xE000));
    ASSERT_FALSE(traits_type::is_recyclable(0xF000));
}

TEST(entity_traits_16b, combine) {
    using traits_type = ecfw::entity_traits<std::uint16_t>;
    std::uint16_t e = traits_type::combine(0xE, 0x00F);
    ASSERT_EQ(e, 0xF00E);
}