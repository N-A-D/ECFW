#include <gtest/gtest.h>
#include <ecfw/world.hpp>
#include <boost/range/adaptor/reversed.hpp>

TEST(view, iteration)
{
    ecfw::world world{};

    world.create<bool>(10);

    auto view = world.view<bool>();

    auto is_active = [](auto data) { return data.get<1>(); };

    auto begin = view.begin();

    ASSERT_EQ(view.size(), 10);

    (void)*begin;

    // ASSERT_TRUE(std::none_of(view.begin(), view.end(), is_active));

    // for (auto&& [e, b] : view)
    //     b = true;

    // ASSERT_TRUE(std::all_of(view.begin(), view.end(), is_active));
}
