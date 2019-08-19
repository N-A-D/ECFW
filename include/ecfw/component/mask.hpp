#pragma once

#include <bitset>

namespace ecfw {
	namespace detail {
		
		template <
			class... Cs
		> using comp_mask = std::bitset<sizeof...(Cs)>;

	}
}