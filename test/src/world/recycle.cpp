#include <iterator>
#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, recycle)
{
    ecfw::world world{};
    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    auto e0 = world.create();
    auto e1 = world.create<int>();
    
    ASSERT_EQ(ecfw::detail::version_from_entity(e0), 0);
    ASSERT_EQ(ecfw::detail::version_from_entity(e1), 0);

    ASSERT_FALSE(applicable.contains(e0));
    ASSERT_FALSE(nonapplicable.contains(e0));

    ASSERT_TRUE(applicable.contains(e1));
    ASSERT_FALSE(nonapplicable.contains(e1));

    world.destroy(e0);
    world.destroy(e1);

    ASSERT_FALSE(applicable.contains(e0));
    ASSERT_FALSE(nonapplicable.contains(e0));

    ASSERT_FALSE(applicable.contains(e1));
    ASSERT_FALSE(nonapplicable.contains(e1));

    e0 = world.create();
    e1 = world.create();

    ASSERT_EQ(ecfw::detail::version_from_entity(e0), 1);
    ASSERT_EQ(ecfw::detail::version_from_entity(e1), 1);

    ASSERT_FALSE(applicable.contains(e0));
    ASSERT_FALSE(nonapplicable.contains(e0));

    ASSERT_FALSE(applicable.contains(e1));
    ASSERT_FALSE(nonapplicable.contains(e1));
}
