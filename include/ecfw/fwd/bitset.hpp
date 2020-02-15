#pragma once

#include <cstddef> // std::size_t
#include <ecfw/aliases/bitset2.hpp>

namespace ecfw {
	namespace detail {

		template <std::size_t N>
		using bitset = bs2::bitset2<N>;

	}
}