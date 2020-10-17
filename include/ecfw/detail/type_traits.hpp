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
	struct is_iterator<T, 
			std::void_t<typename std::iterator_traits<T>::iterator_category>>
		: std::true_type {};

	template <typename T>
	constexpr auto is_iterator_v = is_iterator<T>::value;

	// Using these until I'm able to use C++20

	template <typename T>
	struct is_copyable : 
		std::conjunction<
			std::is_copy_constructible<T>, 
			std::is_copy_assignable<T>,
			std::is_swappable<T>
		> 
	{};

	template <typename T>
	constexpr auto is_copyable_v = is_copyable<T>::value;

	template <typename T>
	struct is_movable : 
		std::conjunction<
			std::is_move_constructible<T>,
			std::is_move_assignable<T>,
			std::is_swappable<T>
		>
	{};

	template <typename T>
	constexpr auto is_movable_v = is_movable<T>::value;

} // namespace detail
} // namespace ecfw
