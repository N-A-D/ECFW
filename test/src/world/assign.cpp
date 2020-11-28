#include <iterator>
#include <type_traits>
#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, assign) {
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    ASSERT_EQ(applicable.size(), 0);
    ASSERT_EQ(nonapplicable.size(), 0);

    auto entity = world.create();

    ASSERT_FALSE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));

    auto& result = world.assign<int>(entity, 1);

    ASSERT_EQ(applicable.size(), 1);
    ASSERT_EQ(nonapplicable.size(), 0);

    ASSERT_TRUE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));

    ASSERT_TRUE((std::is_same_v<int&, decltype(result)>));
    ASSERT_EQ(result, 1);
}

TEST(world, range_assign) {
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    ASSERT_EQ(applicable.size(), 0);
    ASSERT_EQ(nonapplicable.size(), 0);

    uint64_t entities[10];
    world.create(std::begin(entities), std::end(entities));
    for (auto entity : entities) {
        ASSERT_FALSE(applicable.contains(entity));
        ASSERT_FALSE(nonapplicable.contains(entity));

        auto& result = world.assign<int>(entity, 1);

        ASSERT_TRUE(applicable.contains(entity));
        ASSERT_FALSE(nonapplicable.contains(entity));

        ASSERT_TRUE((std::is_same_v<int&, decltype(result)>));
        ASSERT_EQ(result, 1);
    }
    ASSERT_EQ(applicable.size(), std::size(entities));
    ASSERT_EQ(nonapplicable.size(), 0);
}

TEST(world, assign_or_replace) {
    ecfw::world world{};

    auto applicable = world.view<int>();
    auto nonapplicable = world.view<char>();

    auto entity = world.create();

    ASSERT_EQ(applicable.size(), 0);
    ASSERT_EQ(nonapplicable.size(), 0);

    ASSERT_FALSE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));

    auto& result = world.assign<int>(entity, 1);

    ASSERT_EQ(applicable.size(), 1);
    ASSERT_EQ(nonapplicable.size(), 0);

    ASSERT_TRUE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));

    ASSERT_TRUE((std::is_same_v<int&, decltype(result)>));
    ASSERT_EQ(result, 1);

    result = world.assign_or_replace<int>(entity, 2);
    ASSERT_EQ(result, 2);

    ASSERT_EQ(applicable.size(), 1);
    ASSERT_EQ(nonapplicable.size(), 0);

    ASSERT_TRUE(applicable.contains(entity));
    ASSERT_FALSE(nonapplicable.contains(entity));
}
