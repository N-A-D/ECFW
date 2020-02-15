#pragma once

#include <ecfw/aliases/bitset2.hpp>

namespace ecfw {
	namespace detail {

		template <std::size_t N>
		using bitset = bs2::bitset2<N>;

	}
}