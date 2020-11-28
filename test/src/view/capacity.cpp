#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(view, empty)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();
    
    world.create<int>(10);

    ASSERT_FALSE(applicable.empty());
    ASSERT_TRUE(nonapplicable.empty());
}

TEST(view, size)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();
    
    world.create<int>(10);

    ASSERT_EQ(applicable.size(), 10);
    ASSERT_EQ(nonapplicable.size(), 0);
}