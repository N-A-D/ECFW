#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, valid)
{
    ecfw::world world{};

    auto valid_entity = world.create();

    ASSERT_TRUE(world.valid(valid_entity));

    uint64_t invalid_entity = 0xDEADBEEF;

    ASSERT_FALSE(world.valid(invalid_entity));
}