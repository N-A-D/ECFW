#include <gtest/gtest.h>
#include <ecfw/component/mask.hpp>

struct Stub {
	static constexpr auto xs = ecfw::hana::make_tuple(ecfw::hana::type_c<int>, ecfw::hana::type_c<char>, ecfw::hana::type_c<float>, ecfw::hana::type_c<double>);
	
	bool function() const {
		if constexpr (ecfw::detail::bitset<4>(6) == ecfw::detail::make_mask(Stub::xs, ecfw::hana::make_tuple(ecfw::hana::type_c<char>, ecfw::hana::type_c<float>)))
			return true;
		else
			return false;
	}
};

TEST(ComponentMaskTests, MakeMask) {
	constexpr auto xs = ecfw::hana::make_tuple(ecfw::hana::type_c<int>, ecfw::hana::type_c<char>, ecfw::hana::type_c<float>, ecfw::hana::type_c<double>);
	constexpr auto ys = ecfw::hana::make_tuple(ecfw::hana::type_c<int>, ecfw::hana::type_c<float>, ecfw::hana::type_c<double>);
	if constexpr (ecfw::detail::bitset<4>(13) == ecfw::detail::make_mask(xs, ys))
		ASSERT_TRUE(true);
	else
		ASSERT_TRUE(false);
	Stub stub;
	ASSERT_TRUE(stub.function());
}
