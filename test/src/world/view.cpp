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

    auto e1 = world.create<int, char>();
    auto e2 = world.create<int, char>();
    
    ASSERT_TRUE(v0.contains(e1));
    ASSERT_TRUE(v0.contains(e2));

    ASSERT_TRUE(v1.contains(e1));
    ASSERT_TRUE(v1.contains(e2));

    ASSERT_TRUE(v2.contains(e1));
    ASSERT_TRUE(v2.contains(e2));

    ASSERT_TRUE(v3.contains(e1));
    ASSERT_TRUE(v3.contains(e2));

}