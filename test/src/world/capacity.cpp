#include <vector>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, size)
{
    ecfw::world world{};
    
    auto entity = world.create<int>();

    ASSERT_EQ(world.size<int>(), 1);

    world.orphan(entity);

    // While the entity no longer has the component, the component itself is
    // not deallocated and remains to be reused.
    ASSERT_EQ(world.size<int>(), 1);
}

TEST(world, empty)
{
    ecfw::world world{};

    auto entity = world.create<int>();

    ASSERT_FALSE(world.empty<int>());

    world.orphan(entity);

    // While the entity no longer has the component, the component itself is
    // not deallocated and remains to be reused.
    ASSERT_FALSE(world.empty<int>());
}

TEST(world, shrink_to_fit)
{
    ecfw::world world{};
    world.reserve<int>(10);
    ASSERT_EQ(world.capacity<int>(), 10);
    world.shrink_to_fit<int>();
    ASSERT_EQ(world.capacity<int>(), 0);

    world.reserve<char, float>(10);
    ASSERT_EQ(world.capacity<char>(), 10);
    ASSERT_EQ(world.capacity<float>(), 10);
    world.shrink_to_fit<char, float>();
    ASSERT_EQ(world.capacity<char>(), 0);
    ASSERT_EQ(world.capacity<float>(), 0);
}

TEST(world, reserve)
{
    ecfw::world world{};
    world.reserve<int>(10);
    ASSERT_EQ(world.capacity<int>(), 10);

    world.reserve<char, float>(10);
    ASSERT_EQ(world.capacity<char>(), 10);
    ASSERT_EQ(world.capacity<float>(), 10);
}