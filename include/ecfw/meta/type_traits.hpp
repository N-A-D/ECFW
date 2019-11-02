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
		> struct has_type_member
			: std::false_type {};

		template <
			class T
		> struct has_type_member<T, std::void_t<typename T::type>>
			: std::true_type {};

		template <
			class T
		> constexpr bool has_type_member_v = has_type_member<T>::value;

	}
}