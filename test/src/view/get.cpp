#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(view, get)
{
    ecfw::world world{};
    auto entity = world.create<int, char>();
    auto view = world.view<int, char>();
    int& c = view.get<int>(entity);
    ASSERT_EQ(c, 0);
    ASSERT_EQ(std::get<1>(view.get(entity)), 0);
}
