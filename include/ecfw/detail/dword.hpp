#pragma once

#include <cstdint> // uint32_t, uint64_t

namespace ecfw
{
namespace detail
{

	[[nodiscard]] constexpr uint64_t concat(uint32_t msw, uint32_t lsw) noexcept {
		return uint64_t(msw) << 32 | uint64_t(lsw);
	}

	[[nodiscard]] constexpr uint32_t msw(uint64_t dword) noexcept {
		return static_cast<uint32_t>(dword >> 32);
	}

	[[nodiscard]] constexpr uint32_t lsw(uint64_t dword) noexcept {
		return static_cast<uint32_t>(dword & 0xFFFFFFFF);
	}

} // namespace detail
} // namespace ecfw