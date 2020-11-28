#include <gtest/gtest.h>
#include <ecfw/world.hpp>
#include <boost/range/adaptor/reversed.hpp>

TEST(view, forward_iteration)
{
    ecfw::world world{};

    auto view = world.view<bool>();

    world.create<bool>(10);

    auto is_active = [&view](auto e) { return view.get(e); };

    ASSERT_TRUE(std::none_of(view.begin(), view.end(), is_active));

    for (auto e : view)
        view.get(e) = true;

    ASSERT_TRUE(std::all_of(view.begin(), view.end(), is_active));
}

TEST(view, reverse_iteration)
{
    ecfw::world world{};

    auto view = world.view<bool>();

    world.create<bool>(10);

    auto is_active = [&view](auto e) { return view.get(e); };

    ASSERT_TRUE(std::none_of(view.rbegin(), view.rend(), is_active));

    for (auto e : boost::adaptors::reverse(view))
        view.get(e) = true;

    ASSERT_TRUE(std::all_of(view.rbegin(), view.rend(), is_active));
}
