#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(view, contains)
{
    ecfw::world world{};
    auto e0 = world.create<int>();
    auto e1 = world.create<char>();
    auto view = world.view<int>();
    ASSERT_TRUE(view.contains(e0));
    ASSERT_FALSE(view.contains(e1));
}
