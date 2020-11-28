#include <iterator>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, destroy)
{
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    auto entity = world.create<int>();

    ASSERT_TRUE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));

    world.destroy(entity);

    ASSERT_FALSE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));
}

TEST(world, destroy_from_range)
{
    ecfw::world world{};
    uint64_t entities[10];

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    world.create<int>(std::begin(entities), std::end(entities));

    auto is_valid = [&world](auto e) { return world.valid(e); };
    auto is_in_applicable_group = 
        [&applicable](auto e){ return applicable.contains(e); };
    auto is_in_nonapplicable_group = 
        [&nonapplicable](auto e){ return nonapplicable.contains(e); };
        
    ASSERT_TRUE(
        std::all_of(
            std::begin(entities), 
            std::end(entities), 
            is_valid
        )
    );
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
    world.destroy(std::begin(entities), std::end(entities));
    ASSERT_TRUE(
        std::none_of(
            std::begin(entities), 
            std::end(entities), 
            is_valid
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
