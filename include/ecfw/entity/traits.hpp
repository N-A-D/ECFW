#pragma once

#include <cstdint>
#include <cassert>

namespace ecfw {

	template <class>
	struct entity_traits;

	template <>
	struct entity_traits<uint64_t>
	{
		using entity_type  = uint64_t;
		using index_type   = uint32_t;
		using version_type = uint32_t;

		static constexpr index_type max_index = 0xFFFFFFFF;
		static constexpr version_type max_version = 0xFFFFFFFF;

		static constexpr index_type index(entity_type e) noexcept {
			return e & max_index;
		}

		static constexpr version_type version(entity_type e) noexcept {
			return e >> 32;
		}

		static constexpr entity_type create(index_type i, version_type  v) noexcept {
			return entity_type(i) | entity_type(v) << 32;
		}

		static constexpr entity_type recycle(entity_type e) noexcept {
			assert(version(e) < max_version);
			return create(index(e), version(e) + 1);
		}

	};
	
	template <>
	struct entity_traits<uint32_t>
	{
		using entity_type  = uint32_t;
		using index_type   = uint32_t;
		using version_type = uint32_t;

		static constexpr index_type  max_index = 0xFFFFF;
		static constexpr version_type max_version = 0xFFF;

		static constexpr index_type index(entity_type e) noexcept {
			return e & max_index;
		}

		static constexpr version_type version(entity_type e) noexcept {
			return e >> 20;
		}

		static constexpr entity_type create(index_type i, version_type v) noexcept {
			assert(i <= max_index && v <= max_version);
			return entity_type(i) | entity_type(v) << 20;
		}

		static constexpr entity_type recycle(entity_type e) noexcept {
			assert(version(e) < max_version);
			return create(index(e), version(e) + 1);
		}

	};

	template <>
	struct entity_traits<uint16_t>
	{

		using entity_type  = uint16_t;
		using index_type   = uint16_t;
		using version_type = uint16_t;

		static constexpr index_type max_index = 0xFFF;
		static constexpr version_type max_version = 0xF;

		static constexpr index_type index(entity_type e) noexcept {
			return e & max_index;
		}

		static constexpr version_type version(entity_type e) noexcept {
			return e >> 12;
		}

		static constexpr entity_type create(index_type i, version_type v) noexcept {
			assert(i <= max_index && v <= max_version);
			return entity_type(i) | entity_type(v) << 12;
		}

		static constexpr entity_type recycle(entity_type e) noexcept {
			assert(version(e) < max_version);
			return create(index(e), version(e) + 1);
		}

	};
}