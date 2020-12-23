#include <gtest/gtest.h>
#include <ecfw/world.hpp>

TEST(world, contains)
{
    ecfw::world w0{};
    
    ASSERT_FALSE(w0.contains<int>());
    ASSERT_FALSE(w0.contains<char>());

    (void) w0.create<int, char>();

    ASSERT_TRUE(w0.contains<int>());
    ASSERT_TRUE(w0.contains<char>());
    ASSERT_EQ(w0.num_contained_types(), 2);
}  

TEST(world, uniqueness_between_worlds)
{
    ecfw::world w0{};

    ASSERT_FALSE(w0.contains<int>());
    ASSERT_FALSE(w0.contains<char>());

    (void) w0.create<int, char>();

    ASSERT_TRUE(w0.contains<int>());
    ASSERT_TRUE(w0.contains<char>());
    ASSERT_TRUE((w0.contains<int, char>()));

    ASSERT_EQ(w0.num_contained_types(), 2);

    ecfw::world w1{};
    
    ASSERT_FALSE(w1.contains<int>());
    ASSERT_FALSE(w1.contains<char>());
    ASSERT_FALSE((w1.contains<int, char>()));
    ASSERT_EQ(w1.num_contained_types(), 0);
}