#pragma once

#include <utility> // std::pair

#include <ecfw/fwd/unsigned.hpp>

namespace ecfw {
	namespace detail {
		
		/**
		 * @brief Assembles a new entity identifier.
		 * 
		 * @param v The index version.
		 * @param i The row into a component matrix.
		 * @return A new entity identifier.
		 */
		constexpr u64 make_eid(u32 v, u32 i) noexcept {
			return u64(v) << 32 | u64(i);
		}

		/**
		 * @brief Disassembles an entity identifier.
		 * 
		 * @param eid An entity identifier to disassemble.
		 * @return A std::pair of index version and index.
		 */
		constexpr std::pair<u32, u32> unpack_eid(u64 eid) noexcept {
			u32 v = static_cast<u32>(eid >> 32);
			u32 i = static_cast<u32>(eid & 0xffffffff);
			return std::make_pair(v, i);
		}

	}
}