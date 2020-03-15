#pragma once

#include <ecfw/fwd/vector.hpp>

namespace ecfw {

	/**
	 * @brief A trait that specifies the container for another type.
	 * 
	 * The default storage container for all components is
	 * ecfw::block_vector. The ecfw::block_vector is a simple
	 * array-like type which logically allocates enough space
	 * to accommodate the entity with the largest index. As a
	 * result, there can be unused but allocated memory regions
	 * when only a limited number of entities own a component
	 * stored in the block_vector.
	 * 
	 * This trait can be  specialized to indicate another means 
	 * to storage a single component.
	 * 
	 * @note The specialization of this trait requires that it
	 * includes a member type: `type`. The code sample that 
	 * follows depicts a typical scenario where this trait
	 * is specialized to indicate a storage container for a
	 * component type.
	 * 
	 * @code
	 * struct MyComponent {
	 * 		...
	 * };
	 * 
	 * template <typename T>
	 * struct CustomStorageImpl {
	 * 		T* at(size_type);
	 *      const T* at(size_type) const;
	 *      template <typename... Args>
	 *      T* construct(size_type, Args&&...);
	 * 		void destroy(size_type);
	 * };
	 * 
	 * namespace ecfw {
	 * 		template <>
	 * 		struct storage<MyComponent> {
	 * 			using type = CustomStorageImpl<MyComponent>;
	 * 		}; 
	 * }
	 * @endcode 
	 * 
	 * @tparam T The candidate component type.
	 */
	template <typename T>
	struct storage {
		using type = block_vector<T>;
	};

	template <typename T>
	using storage_t = typename storage<T>::type;

}