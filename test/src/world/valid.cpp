#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, is_valid)
{
    ecfw::world world{};

    auto valid_entity = world.create();

    ASSERT_TRUE(world.is_valid(valid_entity));

    uint64_t invalid_entity = 0xDEADBEEF;

    ASSERT_FALSE(world.is_valid(invalid_entity));
}