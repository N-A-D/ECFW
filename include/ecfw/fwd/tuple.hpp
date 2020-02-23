#pragma once

#include <ecfw/fwd/tag.hpp>

namespace ecfw {

	template <typename... Xs>
	using tuple = hana::tuple<tag<Xs>...>;
	
}