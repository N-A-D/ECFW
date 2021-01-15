#include <iterator>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, clone)
{
    ecfw::world world{};
    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();
    auto progenitor = world.create<int>();
    ASSERT_EQ(applicable.size(), 1);
    ASSERT_EQ(nonapplicable.size(), 0);
    ASSERT_TRUE(applicable.contains(progenitor));
    auto clone = world.clone<int>(progenitor);
    ASSERT_NE(progenitor, clone);
    ASSERT_EQ(applicable.size(), 2);
    ASSERT_EQ(nonapplicable.size(), 0);
    ASSERT_TRUE(applicable.contains(progenitor));
    ASSERT_TRUE(applicable.contains(clone));
}

TEST(world, n_clone)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    auto progenitor = world.create<int>();

    ASSERT_EQ(applicable.size(), 1);
    ASSERT_EQ(nonapplicable.size(), 0);
    ASSERT_TRUE(applicable.contains(progenitor));

    world.clone<int>(progenitor, 10);

    ASSERT_EQ(applicable.size(), 11);
    ASSERT_EQ(nonapplicable.size(), 0);
}

TEST(world, first_n_of_range_clone)
{
    ecfw::world world{};
    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();
    auto is_in_applicable_group = 
        [&applicable] (auto e) { return applicable.contains(e); };
    auto is_in_nonapplicable_group = 
        [&nonapplicable] (auto e) { return nonapplicable.contains(e); };
    auto progenitor = world.create<int>();
    ASSERT_TRUE(applicable.contains(progenitor));
    ASSERT_FALSE(nonapplicable.contains(progenitor));
    uint32_t entities[10];
    world.clone<int>(progenitor, std::begin(entities), std::size(entities));
    ASSERT_TRUE(
        std::all_of(
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

TEST(world, range_clone)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    auto is_in_applicable_group = 
        [&applicable] (auto e) { return applicable.contains(e); };
    auto is_in_nonapplicable_group = 
        [&nonapplicable] (auto e) { return nonapplicable.contains(e); };

    auto progenitor = world.create<int>();

    ASSERT_TRUE(applicable.contains(progenitor));
    ASSERT_FALSE(nonapplicable.contains(progenitor));
    
    uint32_t entities[10];
    world.clone<int>(progenitor, std::begin(entities), std::end(entities));
    ASSERT_TRUE(
        std::all_of(
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