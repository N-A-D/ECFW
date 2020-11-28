#include <iterator>
#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, remove)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    auto e = world.create<int>();

    ASSERT_TRUE(applicable.contains(e));
    ASSERT_FALSE(nonapplicable.contains(e));
    ASSERT_TRUE(world.has<int>(e));

    world.remove<int>(e);

    ASSERT_FALSE(world.has<int>(e));
    ASSERT_FALSE(applicable.contains(e));
    ASSERT_FALSE(nonapplicable.contains(e));
}

TEST(world, range_remove)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    uint64_t entities[10];

    world.create<int>(std::begin(entities), std::end(entities));

    auto has_component = [&world](auto e) { return world.has<int>(e); };
    auto is_in_applicable_group = 
        [&applicable](auto e) { return applicable.contains(e); };
    auto is_in_nonapplicable_group = 
        [&nonapplicable](auto e) { return nonapplicable.contains(e); };
    ASSERT_TRUE(
        std::all_of(
            std::begin(entities), 
            std::end(entities), 
            has_component
        )
    );
    ASSERT_TRUE(
        std::all_of(
            std::begin(entities), 
            std::end(entities), 
            is_in_applicable_group));
    ASSERT_TRUE(
        std::none_of(
            std::begin(entities), 
            std::end(entities), 
            is_in_nonapplicable_group
        )
    );
    world.remove<int>(std::begin(entities), std::end(entities));
    ASSERT_TRUE(
        std::none_of(std::begin(entities), 
        std::end(entities), 
        has_component
        )
    );
    ASSERT_TRUE(
        std::none_of(
            std::begin(entities), 
            std::end(entities), 
            is_in_applicable_group
        )
    );
    ASSERT_TRUE(
        std::none_of(
            std::begin(entities), 
            std::end(entities), 
            is_in_nonapplicable_group
        )
    );
}