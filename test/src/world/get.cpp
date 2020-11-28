#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, get)
{
    ecfw::world world{};

    auto entity = world.create<int, bool>();

    ASSERT_EQ(world.get<int>(entity), 0);
    ASSERT_FALSE((world.get<bool>(entity)));
}