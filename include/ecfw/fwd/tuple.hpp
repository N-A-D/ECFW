#pragma once

#include <ecfw/aliases/hana.hpp>

namespace ecfw {

	template <typename... Xs>
	using tuple = hana::tuple<Xs...>;

	using hana::make_tuple;

}