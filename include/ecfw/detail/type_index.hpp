#pragma once

#include <cstddef>
#include <functional>
#include <boost/type_index/ctti_type_index.hpp>

namespace ecfw
{
namespace detail
{

    using type_index = boost::typeindex::ctti_type_index;

} // namespace detail
} // namespace ecfw

namespace std
{
    template <>
    struct hash<boost::typeindex::ctti_type_index> {
        std::size_t 
        operator()(const boost::typeindex::ctti_type_index& t) const noexcept {
            return t.hash_code();
        }
    };
} // namespace std