#pragma once

#include <vector> // vector
#include <utility> // is_const

namespace ecfw 
{
namespace detail 
{

	template <typename C>
	using buffer_type =
		std::conditional_t<std::is_const_v<C>, const std::vector<C>, std::vector<C>>;

} // namespace detail
} // namespace ecfw