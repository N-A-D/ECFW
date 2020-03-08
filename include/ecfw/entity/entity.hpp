#pragma once

#include <utility> // pair
#include <ecfw/fwd/unsigned.hpp>

namespace ecfw { namespace detail {

	/**
	 * @brief Creates a new entity.
	 * 
	 * @param v The entitys version.
	 * @param i The entitys index.
	 * @return The newly created entity.
	 */
	u64 make_entity(u32 v, u32 i) noexcept {
		return u64(v) << 32 | u64(i);
	}

	/**
	 * @brief Disassembles an entity.
	 * 
	 * @param entity The entity to disassemble.
	 * @return A pair whose values are the entitys version and index.
	 */
	std::pair<u32, u32> unpack_entity(u64 entity) noexcept {
		u32 v = static_cast<u32>(entity >> 32);
		u32 i = static_cast<u32>(entity & 0xffffffff);
		return { v, i };
	}

} }