#pragma once

#include <iterator>    // iterator_traits
#include <type_traits> // true_type, false_type, void_t

namespace ecfw
{
namespace detail
{

	template <typename T, typename = std::void_t<>>
	struct is_iterator : std::false_type {};

	template <typename T>
	struct is_iterator<T, std::void_t<typename std::iterator_traits<T>::iterator_category>>
		: std::true_type {};

	template <typename T>
	constexpr auto is_iterator_v = is_iterator<T>::value;

} // namespace detail
} // namespace ecfw
