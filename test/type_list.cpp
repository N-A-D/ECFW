#include <gtest/gtest.h>
#include <ecfw/meta/type_list.hpp>

using namespace ecfw;

TEST(TypeListTests, MetaFunctionTests) {

	static_assert(meta::contains_v<int, type_list<int, char, bool, double>>);
	static_assert(meta::contains_v<int, type_list<char, int, bool, double>>);
	static_assert(meta::contains_v<int, type_list<char, bool, int, double>>);
	static_assert(meta::contains_v<int, type_list<char, bool, double, int>>);
	static_assert(!meta::contains_v<float, type_list<char, bool, double, int>>);

	static_assert(meta::is_unique_v<type_list<int, char, bool, double>>);
	static_assert(!meta::is_unique_v<type_list<int, char, char, double>>);
	static_assert(!meta::is_unique_v<type_list<int, float, char, char>>);

	static_assert(meta::is_subset_v<type_list<int, char>, type_list<char, int, double, float>>);
	static_assert(meta::is_subset_v<type_list<int, char>, type_list<char, double, int, float>>);
	static_assert(meta::is_subset_v<type_list<int, char>, type_list<char, double, float, int>>);

	static_assert(meta::index_of_v<int, type_list<int, char, double, float>> == 0);
	static_assert(meta::index_of_v<char, type_list<int, char, double, float>> == 1);
	static_assert(meta::index_of_v<double, type_list<int, char, double, float>> == 2);
	static_assert(meta::index_of_v<float, type_list<int, char, double, float>> == 3);

}