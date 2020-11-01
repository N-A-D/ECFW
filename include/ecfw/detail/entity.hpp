#pragma once

#include <cstdint> // uint32_t, uint64_t

namespace ecfw
{
namespace detail
{

    [[nodiscard]] 
    constexpr uint64_t make_entity(uint32_t version, uint32_t index) noexcept {
        return uint64_t(version) << 32 | uint64_t(index);
    }

    [[nodiscard]] 
    constexpr uint32_t version(uint64_t entity) noexcept {
        return static_cast<uint32_t>(entity >> 32);
    }

    [[nodiscard]] 
    constexpr uint32_t index(uint64_t entity) noexcept {
        return static_cast<uint32_t>(entity & 0xFFFFFFFF);
    }

} // namespace detail
} // namespace ecfw