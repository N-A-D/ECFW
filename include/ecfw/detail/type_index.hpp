#pragma once

#include <cstddef> // size_t
#include <type_traits>

namespace ecfw
{
namespace detail
{

	struct counter {
		static size_t increment() noexcept {
			static size_t count{ 0 };
			return count++;
		}
	};

	template <typename T>
	struct type_index {
		inline static const size_t value = counter::increment();
	};

	template <typename T>
	size_t type_index_v = detail::type_index<std::decay_t<T>>::value;

} // namespace detail
} // namespace 