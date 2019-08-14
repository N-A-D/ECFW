#pragma once

#include <ancillary/container/bitset.hpp>

namespace ecfw {
	namespace detail {
		
		template <
			class... Cs
		> using comp_mask = ancillary::bitset<sizeof...(Cs)>;

	}
}