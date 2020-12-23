#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, has)
{
    ecfw::world world{};

    auto e0 = world.create();
    auto e1 = world.create<int, bool>();

    ASSERT_FALSE(world.has<int>(e0));
    ASSERT_TRUE(world.has<int>(e1));
    ASSERT_TRUE(world.has<bool>(e1));
    ASSERT_TRUE((world.has<int, bool>(e1)));
}