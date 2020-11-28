#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, construction_consistency)
{
    ecfw::world world{};
    ecfw::view<int, char> v0 = world.view<int, char>();
    ecfw::view<const int, char> v1 = world.view<const int, char>();
    ecfw::view<int, const char> v2 = world.view<int, const char>();
    ecfw::view<const int, const char> v3 = world.view<const int, const char>();

    world.create<int, char>(2);

    ASSERT_TRUE(std::equal(v0.begin(), v0.end(), v1.begin(), v1.end()));
    ASSERT_TRUE(std::equal(v1.begin(), v1.end(), v2.begin(), v2.end()));
    ASSERT_TRUE(std::equal(v2.begin(), v2.end(), v3.begin(), v3.end()));
}