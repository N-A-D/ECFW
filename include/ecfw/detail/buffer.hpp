#pragma once

#include <vector> // vector
#include <utility> // is_const
#include <type_traits>

namespace ecfw 
{
namespace detail 
{

	template <typename C>
	using buffer_type =
		std::conditional_t<std::is_const_v<C>, const std::vector<std::decay_t<C>>, std::vector<std::decay_t<C>>>;

} // namespace detail
} // namespace ecfw