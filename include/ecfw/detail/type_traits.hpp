#pragma once

#include <iterator>
#include <type_traits>

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
