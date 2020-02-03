#pragma once

namespace ecfw {
	namespace detail {

		/**
		 * @brief Encodes the presence of types from one sequence in another.
		 * 
		 * The purpose of this function is to create compile time bitsets
		 * for grouping entities together.
		 * 
		 * @pre The sequence of types to encode as 'set' must be members of the
		 * larger sequence.
		 * 
		 * @tparam Xs An iteraable sequence type.
		 * @tparam Ys An iterable sequence type.
		 * @param xs A sequence of types whose indices make up bits in a bitset.
		 * @param ys A sequence of types to encode as 'set'.
		 * @return A bitset whose active bits indicates present types.
		 */
		template <typename Xs, typename Ys>
		constexpr auto make_mask(const Xs& xs, const Ys& ys);

	}
}