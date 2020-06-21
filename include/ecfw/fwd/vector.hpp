#pragma once

#include <cstddef>

namespace ecfw {

	template <
		typename T, 
		std::size_t BlockSize = 4096
	> class block_vector;

}