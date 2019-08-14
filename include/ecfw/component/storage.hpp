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

		using container_type  = std::vector<Component, Allocator>;
		using value_type      = typename container_type::value_type;
		using size_type       = typename container_type::size_type;
		using reference       = typename container_type::reference;
		using const_reference = typename container_type::const_reference;

		reference get(size_type index) {
			assert(index < size());
			return m_data[index];
		}

		const_reference get(size_type index) const {
			assert(index < size());
			return m_data[index];
		}

		bool empty() const noexcept { return m_data.empty(); }
		size_type size() const noexcept { return m_data.size(); }
		void reserve(size_type cap) { m_data.reserve(cap); }

		template <class... Args>
		reference construct(size_type index, Args&&... args) {
			if (index >= size())
				m_data.resize(index + 1, Component());
			Component comp(std::forward<Args>(args)...);
			if constexpr (std::is_move_assignable_v<Component>)
				return (m_data[index] = std::move(comp));
			else
				return (m_data[index] = comp);
		}

		void destroy(size_type index) {}

		void clear() noexcept {
			m_data.clear();
		}

	private:
		container_type m_data;
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