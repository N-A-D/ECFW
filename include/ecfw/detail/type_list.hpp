#pragma once

#include <boost/hana.hpp> // basic_type, tuple

namespace ecfw {
namespace detail {

	template <typename T>
	using type = boost::hana::basic_type<T>;

	template <typename T>
	constexpr auto type_v = type<T>{};

	template <typename... Ts>
	using type_list = boost::hana::tuple<boost::hana::basic_type<Ts>...>;

	template <typename... Ts>
	constexpr auto type_list_v = type_list<Ts...>{};

} // namespace detail
} // namespace ecfw