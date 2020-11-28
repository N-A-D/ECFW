#include <vector>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, max_size)
{
    ecfw::world world{};
    
    // We need to create entity here because the world
    // does not yet have vector storage for type int
    (void) world.create<int>();

    ASSERT_EQ(world.max_size<int>(), std::vector<int>{}.max_size());
}

TEST(world, size)
{
    ecfw::world world{};
    
    auto entity = world.create<int>();

    ASSERT_EQ(world.size<int>(), 1);

    world.orphan(entity);

    ASSERT_EQ(world.size<int>(), 0);
}

TEST(world, empty)
{
    ecfw::world world{};

    auto entity = world.create<int>();

    ASSERT_FALSE(world.empty<int>());

    world.orphan(entity);

    ASSERT_TRUE(world.empty<int>());
}

TEST(world, shrink_to_fit)
{
    ecfw::world world{};
    world.reserve<int>(10);
    ASSERT_EQ(world.capacity<int>(), 10);
    world.shrink_to_fit<int>();
    ASSERT_EQ(world.capacity<int>(), 0);
}

TEST(world, reserve)
{
    ecfw::world world{};
    world.reserve<int>(10);
    ASSERT_EQ(world.capacity<int>(), 10);
}