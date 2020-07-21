#include <gtest/gtest.h>
#include <ecfw/detail/type_index.hpp>

TEST(type_index, type_index)
{
	namespace dtl = ecfw::detail;

	struct T0 {};
	struct T1 {};
	struct T2 {};

	ASSERT_EQ(dtl::type_index_v<T0>, dtl::type_index_v<T0>);
	ASSERT_LT(dtl::type_index_v<T0>, dtl::type_index_v<T1>);
	ASSERT_GT(dtl::type_index_v<T2>, dtl::type_index_v<T1>);
}