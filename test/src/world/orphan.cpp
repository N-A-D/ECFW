#include <iterator>
#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, orphan)
{
    ecfw::world world{};

    auto applicable = world.view<int, char>();
    auto nonapplicable = world.view<float>();

    auto entity = world.create<int, char>();

    ASSERT_TRUE((world.has<int, char>(entity)));
    ASSERT_TRUE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));

    world.orphan(entity);

    ASSERT_FALSE((world.has<int, char>(entity)));
    ASSERT_FALSE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));
}

TEST(world, orphan_from_range)
{
    ecfw::world world{};

    auto applicable = world.view<int, char>();
    auto nonapplicable = world.view<float>();

    uint32_t entities[10];

    world.create<int, char>(std::begin(entities), std::end(entities));

    auto is_in_applicable_group =
        [&applicable] (auto e) { return applicable.contains(e); };
    auto is_in_nonapplicable_group = 
        [&nonapplicable] (auto e) { return nonapplicable.contains(e); };
    auto has_components = 
        [&world] (auto e) { return world.has<int, char>(e); };
    
    ASSERT_TRUE(
        std::all_of(std::begin(entities), 
        std::end(entities), 
        has_components
        )
    );
    ASSERT_TRUE(
        std::all_of(std::begin(entities), 
        std::end(entities),
        is_in_applicable_group
        )
    );
    ASSERT_TRUE(
        std::none_of(std::begin(entities), 
        std::end(entities), 
        is_in_nonapplicable_group
        )
    );
    world.orphan(std::begin(entities), std::end(entities));
    ASSERT_TRUE(
        std::none_of(std::begin(entities), 
        std::end(entities), 
        has_components
        )
    );
    ASSERT_TRUE(
        std::none_of(std::begin(entities), 
        std::end(entities), 
        is_in_applicable_group
        )
    );
    ASSERT_TRUE(
        std::none_of(std::begin(entities), 
        std::end(entities), 
        is_in_nonapplicable_group
        )
    );
}
