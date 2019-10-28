#pragma once

#include <type_traits>
#include <iterator>
#include <ecfw/component/storage.hpp>

namespace ecfw {
	namespace meta {

		template <
			class It,
			class = void
		> struct is_iterator
			: std::false_type {};

		template <
			class It
		> struct is_iterator<It, std::void_t<typename std::iterator_traits<It>::iterator_category>>
			: std::true_type {};

		template <
			class It
		> constexpr bool is_iterator_v = is_iterator<It>::value;

		template <
			class T,
			class = void
		> struct underlying_storage_has_type_member
			: std::false_type {};

		template <
			class T
		> struct underlying_storage_has_type_member<T, std::void_t<typename underlying_storage<T>::type>>
			: std::true_type {};

		template <
			class T
		> constexpr bool underlying_storage_has_type_member_v = underlying_storage_has_type_member<T>::value;

	}
}