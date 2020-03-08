#pragma once

#include <ecfw/fwd/vector.hpp>

namespace ecfw {

	template <typename T>
	struct storage {
		using type = paged_vector<T>;
	};

	template <typename T>
	using storage_t = typename storage<T>::type;

}