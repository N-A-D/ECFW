#include <gtest/gtest.h>
#include <ecfw/detail/type_index.hpp>

struct T0 {};
struct T1 {};
struct T2 {};

TEST(type_index, generation_order) {
	namespace dtl = ecfw::detail;
	ASSERT_EQ(dtl::type_index_v<T0>, 0);
	ASSERT_EQ(dtl::type_index_v<T1>, 1);
	ASSERT_EQ(dtl::type_index_v<T2>, 2);
}