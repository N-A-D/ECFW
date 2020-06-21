#pragma once

#include <type_traits>
#include <iterator>

namespace ecfw { namespace detail {
		
	// Default template as a fall back when instantiation fails
	template <
		typename T, 
		typename = std::void_t<>
	> struct is_iterator : std::false_type {};

	// Tries to instantiate void_t with the iterator_category of the type
	template <
		typename T
	> struct is_iterator
		<T, std::void_t<typename std::iterator_traits<T>::iterator_category>>
		: std::true_type {};

	template <
		typename T
	> constexpr bool is_iterator_v = is_iterator<T>::value;

} }