#include <iterator>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, entity_stats)
{
    ecfw::world world{};
    uint32_t entities[10];

    world.create(std::begin(entities), std::end(entities));

    ASSERT_EQ(world.num_entities(), std::size(entities));
    ASSERT_EQ(world.num_alive(), std::size(entities));
    ASSERT_EQ(world.num_inactive(), 0);

    world.destroy(std::begin(entities), std::end(entities));

    ASSERT_EQ(world.num_entities(), std::size(entities));
    ASSERT_EQ(world.num_alive(), 0);
    ASSERT_EQ(world.num_inactive(), std::size(entities));
}

TEST(world, component_stats)
{
    ecfw::world world{};

    world.create<int>(10);
    world.create<int, char>(10);

    ASSERT_EQ(world.count<int>(), 20);
    ASSERT_EQ((world.count<int, char>()), 10);
}