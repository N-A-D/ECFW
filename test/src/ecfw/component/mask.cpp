#include <gtest/gtest.h>
#include <ecfw/fwd/tuple.hpp>
#include <ecfw/component/mask.hpp>

using namespace ecfw;
using namespace ecfw::detail;

struct Stub {
	static constexpr auto xs = ecfw::tuple<int, char, float, double>{};
	
	bool function() const {
		if constexpr (bitset<4>(6) == make_mask(Stub::xs, tuple<char, float>{}))
			return true;
		else
			return false;
	}
};

TEST(ComponentMaskTests, MakeMask) {
	constexpr auto xs = ecfw::tuple<int, char, float, double>{};
	constexpr auto ys = ecfw::tuple<int, float, double>{};
	if constexpr (bitset<4>(13) == make_mask(xs, ys))
		ASSERT_TRUE(true);
	else
		ASSERT_TRUE(false);
	Stub stub;
	ASSERT_TRUE(stub.function());
}
