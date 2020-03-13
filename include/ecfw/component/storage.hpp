#pragma once

#include <ecfw/fwd/vector.hpp>

namespace ecfw {

	template <typename T>
	struct storage {
		using type = block_vector<T>;
	};

	template <typename T>
	using storage_t = typename storage<T>::type;

}