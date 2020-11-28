#include <gtest/gtest.h>
#include <ecfw/detail/type_traits.hpp>

namespace dtl = ecfw::detail;

TEST(type_traits, is_iterator)
{
	struct iterator {};

	ASSERT_TRUE(dtl::is_iterator_v<std::vector<int>::iterator>);
	ASSERT_FALSE(dtl::is_iterator_v<iterator>);
}

TEST(type_traits, is_copyable)
{
	struct copyable {};
	struct only_copyable {
		only_copyable(const only_copyable&) = default;
		only_copyable& operator=(const only_copyable&) = default;
	};
	struct noncopyable {
		noncopyable(const noncopyable&) = delete;
		noncopyable& operator=(const noncopyable&) = delete;
	};

	ASSERT_TRUE(dtl::is_copyable_v<copyable>);
	ASSERT_TRUE(dtl::is_copyable_v<only_copyable>);
	ASSERT_FALSE(dtl::is_copyable_v<noncopyable>);
}	

TEST(type_traits, is_movable)
{
	struct movable {};
	struct only_movable {
		only_movable(only_movable&&) = default;
		only_movable& operator=(only_movable&&) = default;
	};
	struct nonmovable {
		nonmovable(nonmovable&&) = delete;
		nonmovable& operator=(nonmovable&&) = delete;
	};
	ASSERT_TRUE(dtl::is_movable_v<movable>);
	ASSERT_TRUE(dtl::is_movable_v<only_movable>);
	ASSERT_FALSE(dtl::is_movable_v<nonmovable>);	
}