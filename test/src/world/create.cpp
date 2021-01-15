#include <iterator>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, create)
{
    ecfw::world world;
    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();
    
    auto e0 = world.create();
    ASSERT_TRUE(world.is_valid(e0));
    ASSERT_EQ(applicable.size(), 0);
    ASSERT_EQ(nonapplicable.size(), 0);

    auto e1 = world.create<int>();
    ASSERT_TRUE(world.is_valid(e1));
    ASSERT_EQ(applicable.size(), 1);
    ASSERT_EQ(nonapplicable.size(), 0);
    ASSERT_EQ(world.num_entities(), 2);
    ASSERT_EQ(world.num_alive(), 2);
}

TEST(world, n_create)
{
    ecfw::world world{};
    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();
    world.create<int>(10);
    world.create<char>(10);
    world.create<int, char>(10);
    ASSERT_EQ(applicable.size(), 20);
    ASSERT_EQ(nonapplicable.size(), 20);
    ASSERT_EQ(world.num_entities(), 30);
}

TEST(world, first_n_of_range_clone)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    auto is_in_applicable_group = 
        [&applicable](auto e){ return applicable.contains(e); };
    auto is_in_nonapplicable_group = 
        [&nonapplicable](auto e){ return nonapplicable.contains(e); };

    uint32_t entities[10];
    world.create<int>(std::begin(entities), std::size(entities));
    ASSERT_EQ(world.num_entities(), std::size(entities));
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
        [&applicable](auto e){ return applicable.contains(e); };
    auto is_in_nonapplicable_group = 
        [&nonapplicable](auto e){ return nonapplicable.contains(e); };

    uint32_t entities[10];
    world.create<int>(std::begin(entities), std::end(entities));
    ASSERT_EQ(world.num_entities(), std::size(entities));
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