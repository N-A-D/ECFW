#pragma once

#include <vector> // vector
#include <utility> // is_const
#include <type_traits>
#include <boost/dynamic_bitset.hpp>

namespace ecfw 
{
namespace detail 
{

    template <typename C>
    using buffer_type =
        std::conditional_t<
            std::is_const_v<C>, 
            const std::vector<std::decay_t<C>>, std::vector<std::decay_t<C>>>;

    using metabuffer_type = boost::dynamic_bitset<uint64_t>;

} // namespace detail
} // namespace ecfw