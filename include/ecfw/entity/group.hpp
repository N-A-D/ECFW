#pragma once

#include <vector>
#include <cassert>
#include <ecfw/entity/traits.hpp>

namespace ecfw {

	template <
		class Entity
	> class entity_group final {
		using traits_type = entity_traits<Entity>;
		using index_type = typename traits_type::index_type;
	public:

		using entity_type = Entity;
		using size_type = typename std::vector<entity_type>::size_type;
		using const_iterator = typename std::vector<entity_type>::const_iterator;

		const_iterator begin() const noexcept { return m_entities.begin(); }
		const_iterator cbegin() const noexcept { return m_entities.cbegin(); }

		const_iterator end() const noexcept { return m_entities.end(); }
		const_iterator cend() const noexcept { return m_entities.cend(); }

		bool empty() const noexcept { return m_entities.empty(); }
		size_type size() const noexcept { return m_entities.size(); }

		void clear() noexcept { m_entities.clear(); }

		void insert(entity_type e) {
			if (contains(e))
				return;
			auto idx = traits_type::index(e);
			if (idx >= m_redirect.size())
				m_redirect.resize(idx + 1);
			index_type sz = static_cast<index_type>(m_entities.size());
			m_entities.emplace_back(e);
			m_redirect[idx] = sz;
			assert(contains(e));
		}

		void erase(entity_type e) {
			if (!contains(e))
				return;
			auto idx = traits_type::index(e);
			m_entities[m_redirect[idx]] = m_entities.back();
			m_redirect[traits_type::index(m_entities.back())] = m_redirect[idx];
			m_entities.pop_back();
			assert(!contains(e));
		}

		bool contains(entity_type e) const {
			auto idx = traits_type::index(e);
			return idx < m_redirect.size()
				&& m_redirect[idx] < m_entities.size()
				&& m_entities[m_redirect[idx]] == e;
		}

	private:
		std::vector<index_type> m_redirect;  // entity lookup table
		std::vector<entity_type> m_entities; // entities a part of the group
	};

}