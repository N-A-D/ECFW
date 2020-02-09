#pragma once

#include <cstddef>
#include <bitset2/bitset2.hpp>

namespace ecfw {
	namespace detail {
		template <std::size_t N>
		using bitset = Bitset2::bitset2<N>;
	}
}