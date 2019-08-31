#pragma once

#include <vector>
#include <cassert>
#include <memory>
#include <type_traits>

namespace ecfw {

	template <
		class Component,
		class Allocator = std::allocator<Component>
	> class default_storage { // Default component storage type
	public:

		Component& get(size_t index) {
			assert(index < size());
			return m_data[index];
		}

		const Component& get(size_t index) const {
			assert(index < size());
			return m_data[index];
		}

		bool empty() const noexcept { return m_data.empty(); }
		size_t size() const noexcept { return m_data.size(); }
		void reserve(size_t cap) { m_data.reserve(cap); }

		template <class... Args>
		Component& construct(size_t index, Args&&... args) {
			if (index >= size())
				m_data.resize(index + 1, Component());
			Component comp(std::forward<Args>(args)...);
			if constexpr (std::is_move_assignable_v<Component>)
				return (m_data[index] = std::move(comp));
			else
				return (m_data[index] = comp);
		}

		void destroy(size_t index) {}

		void clear() noexcept {
			m_data.clear();
		}

	private:
		std::vector<Component, Allocator> m_data;
	};

	template <
		class Component
	> struct underlying_storage { // Obtains the underlying storage type for a given component
		using type = default_storage<Component>;
	};
	
	template <
		class Component
	> using underlying_storage_t = typename underlying_storage<Component>::type;

}