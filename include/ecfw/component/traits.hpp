#pragma once

#include <ecfw/fwd/vector.hpp>

namespace ecfw {

	/**
	 * @brief A trait that specifies the storage implementation for a compnent.
	 * 
	 * @note To make use of an alternative storage backend for a specific
	 * component type, provide as template specialization.
	 * 
	 * @code
	 * namespace ecfw {
	 *     template <>
	 *     struct storage<Component> {
	 * 	   		using type = CustomStorage<Component>;
	 *     };
	 * }
	 * @endcode
	 * 
	 * @tparam T A component type.
	 */
	template <typename T>
	struct storage { using type = detail::vector<T>; };

	/**
	 * @brief Helper template for ecfw::storage.
	 * 
	 * @tparam T A component type.
	 */
	template <typename T>
	using storage_t = typename storage<T>::type;

}