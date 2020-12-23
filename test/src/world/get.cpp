#include <gtest/gtest.h>
#include <ecfw/world.hpp>
#include <boost/container/vector.hpp>

TEST(world, get)
{
    ecfw::world world{};

    auto entity = world.create<int, bool>();

    ASSERT_EQ(world.get<int>(entity), 0);
    ASSERT_FALSE((world.get<bool>(entity)));

    auto [i, b] = world.get<int, bool>(entity);

    ASSERT_EQ(i, 0);
    ASSERT_FALSE(b);
}