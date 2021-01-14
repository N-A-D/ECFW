#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(view, get)
{
    ecfw::world world{};
    auto entity = world.create<int, char>();
    auto view = world.view<int, char>();
    auto&& [i, c] = view.get(entity);
    ASSERT_EQ(i, 0);
    ASSERT_EQ(c, 0);
}
