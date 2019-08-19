#pragma once

#include <type_traits>
#include <iterator>

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

	}
}