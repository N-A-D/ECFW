#pragma once

#include <typeinfo>
#include <typeindex>
#include <type_traits>

namespace ecfw
{
namespace detail
{

    template <typename T>
    std::type_index type_index() noexcept {
        return std::type_index{typeid(std::decay_t<T>)};
    }

} // namespace detail
} // namespace 