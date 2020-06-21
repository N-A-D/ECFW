#pragma once

#include <type_traits>
#include <ecfw/aliases/hana.hpp>

namespace ecfw {

	namespace dtl = detail;

	template <
		typename T
	> using type = dtl::hana::basic_type<std::decay_t<T>>;

	template <
		typename T
	> constexpr auto type_v = type<T>{};

	template <
		typename T,
		typename... Ts
	> using type_list =
		dtl::hana::tuple<dtl::hana::basic_type<T>, dtl::hana::basic_type<Ts>...>;

	template <
		typename T,
		typename... Ts
	> constexpr auto type_list_v = type_list<T, Ts...>{};

}