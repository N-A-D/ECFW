#pragma once

#include <cstdint>
#include <cassert>

namespace ecfw {

	template <class>
	struct entity_traits;

	template <>
	struct entity_traits<uint64_t>
	{
		static constexpr uint32_t max_index = 0xFFFFFFFF;
		static constexpr uint32_t max_version = 0xFFFFFFFF;

		static constexpr uint32_t index(uint64_t e) noexcept {
			return e & max_index;
		}

		static constexpr uint32_t version(uint64_t e) noexcept {
			return e >> 32;
		}

		static constexpr uint64_t create(uint32_t i, uint32_t v) noexcept {
			return uint64_t(i) | uint64_t(v) << 32;
		}

		static constexpr uint64_t recycle(uint64_t e) noexcept {
			assert(version(e) < max_version);
			return create(index(e), version(e) + 1);
		}

	};
	
	template <>
	struct entity_traits<uint32_t>
	{
		static constexpr uint32_t max_index = 0xFFFFF;
		static constexpr uint32_t max_version = 0xFFF;

		static constexpr uint32_t index(uint32_t e) noexcept {
			return e & max_index;
		}

		static constexpr uint32_t version(uint32_t e) noexcept {
			return e >> 20;
		}

		static constexpr uint32_t create(uint32_t i, uint32_t v) noexcept {
			assert(i <= max_index && v <= max_version);
			return uint32_t(i) | uint32_t(v) << 20;
		}

		static constexpr uint32_t recycle(uint32_t e) noexcept {
			assert(version(e) < max_version);
			return create(index(e), version(e) + 1);
		}

	};

	template <>
	struct entity_traits<uint16_t>
	{
		static constexpr uint16_t max_index = 0xFFF;
		static constexpr uint16_t max_version = 0xF;

		static constexpr uint16_t index(uint16_t e) noexcept {
			return e & max_index;
		}

		static constexpr uint16_t version(uint16_t e) noexcept {
			return e >> 12;
		}

		static constexpr uint16_t create(uint16_t i, uint16_t v) noexcept {
			assert(i <= max_index && v <= max_version);
			return uint16_t(i) | uint16_t(v) << 12;
		}

		static constexpr uint16_t recycle(uint16_t e) noexcept {
			assert(version(e) < max_version);
			return create(index(e), version(e) + 1);
		}

	};

}