#pragma once

#include <ecfw/aliases/hana.hpp>

namespace ecfw {

	template <typename... Xs>
	using tuple = hana::tuple<hana::basic_type<Xs>...>;
	
}