#pragma once

namespace ecfw {
	namespace detail {
		
		/**
		 * @brief Semi contiguous storage container.
		 * 
		 * The purpose of this class is to serve as the default storage
		 * implementation for components.
		 * 
		 * Leaves uninitialized gaps within the buffer for components not
		 * in use.
		 *
		 * @note Provides O(1) lookup. Amortized O(1) insert + delete.
		 * 
		 * @tparam T A component type.
		 */
		template <typename T>
		class vector;

	}
}