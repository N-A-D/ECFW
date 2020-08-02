#pragma once

#include <type_traits>
#include <boost/hana.hpp>

namespace ecfw {
namespace detail {

	template <typename T>
	using type = boost::hana::basic_type<std::decay_t<T>>;

	template <typename T>
	constexpr auto type_v = type<T>{};

	template <typename T, typename... Ts>
	using type_list = boost::hana::tuple<boost::hana::basic_type<T>, boost::hana::basic_type<Ts>...>;

	template <typename T, typename... Ts>
	constexpr auto type_list_v = type_list<T, Ts...>{};

} // namespace detail
} // namespace ecfw