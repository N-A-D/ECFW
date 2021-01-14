#pragma once

#include <memory>
#include <type_traits>
#include <boost/container/vector.hpp>
#include <boost/dynamic_bitset.hpp>

namespace ecfw
{
namespace detail
{
    
    template <typename T>
    using buffer = boost::container::vector<T>;

    using buffer_ptr = std::unique_ptr<void, void(*)(const void*)>;

    // Returns a type-erased unique_ptr to a buffer container.
    template <typename T>
    buffer_ptr make_buffer() {
        auto deleter = [](const void* pointer) {
            delete static_cast<const buffer<T>*>(pointer);
        };
        return buffer_ptr{ new buffer<T>{}, deleter };
    }

} // namespace detail
} // namespace ecfw