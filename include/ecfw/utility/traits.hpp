#pragma once

#include <type_traits>
#include <iterator>

namespace ecfw {
	namespace detail {

		template <typename T, typename = std::void_t<>>
		struct is_iterator : std::false_type {};

		template <typename T>
		struct is_iterator<T, std::void_t<typename std::iterator_traits<T>::iterator_category>>
			: std::true_type {};

		/**
		 * @brief Determines if a type is an iterator.
		 * 
		 * @note A type is determined to be an iterator if it defines a public
		 * member type: iterator_category.
		 * 
		 * @tparam T The type under consideration.
		 */
		template <typename T>
		constexpr bool is_iterator_v = is_iterator<T>::value;

	}
}