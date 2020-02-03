#pragma once

namespace ecfw {
	namespace detail {
		
		/**
		 * @brief Semi contiguous storage container.
		 * 
		 * The purpose of this class is to serve as the default storage
		 * implementation for components.
		 * 
		 * @note Provides O(1) lookup. Amortized O(1) insert + delete.
		 * 
		 * @tparam T 
		 */
		template <typename T>
		class vector;

	}
}