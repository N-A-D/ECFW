#pragma once

#include <map>          // map
#include <tuple>		// tuple, get, forward_as_tuple
#include <stack>		// stack
#include <vector>		// vector
#include <cassert>		// assert
#include <algorithm>    // generate_n, all_of
#include <type_traits>	// conjunction, is_default_constructible, as_const, is_constructible
#include <ecfw/core/view.hpp>
#include <ecfw/detail/buffer.hpp>
#include <ecfw/detail/integers.hpp>
#include <ecfw/detail/sparse_set.hpp>
#include <ecfw/detail/type_index.hpp>
#include <ecfw/detail/type_traits.hpp>
#include <ecfw/detail/dynamic_bitset.hpp>

namespace ecfw 
{
 
	class world final {
	public:

		template <typename... Ts>
		uint64_t create() {
			using std::conjunction_v;
			using std::is_default_constructible;

			static_assert(conjunction_v<is_default_constructible<Ts>...>);

			uint64_t entity{};

			// There exists a reusable entity
			if (!m_free_list.empty()) {
				uint32_t idx = m_free_list.top();
				m_free_list.pop();
				uint32_t ver = m_versions[idx];
				entity = dtl::concat(ver, idx);
			}
			else {
				// Ensure it is still possible to create new entities
				uint32_t idx = static_cast<uint32_t>(m_versions.size());
				assert(idx < 0xFFFFFFFF);

				// Any new entity is effectively a new row in the component table.
				// However, we refrain from actually allocating space in the component
				// metadata and component data buffers until components are assigned.
				entity = idx;
				m_versions.push_back(0);
			}
			if constexpr (sizeof...(Ts) > 0)
				(assign<Ts>(entity), ...);
			return entity;
		}

		template <typename... Ts, typename OutIt>
		void create(OutIt out, size_t n) {
			for (size_t i = 0; i != n; ++i)
				*out++ = create<Ts...>();
		}

		template <typename... Ts>
		uint64_t clone(uint64_t original) {
			using std::conjunction_v;
			using std::is_copy_constructible;

			static_assert(conjunction_v<is_copy_constructible<Ts>...>);

			uint64_t entity = create<T, Ts...>();
			(assign<Ts>(entity, get<Ts>(original)), ...);
			return entity;
		}

		template <typename... Ts, typename OutIt>
		void clone(uint64_t original, OutIt out, size_t n) {
			for (size_t i = 0; i != n; ++i)
				*out++ = clone<Ts...>(original);
		}

		bool valid(uint64_t eid) const {
			uint32_t idx = dtl::lsw(eid);
			uint32_t ver = dtl::msw(eid);
			return idx < m_versions.size()
				&& m_versions[idx] == ver;
		}

		/// <summary>
		/// Removes an entity from the calling world.
		/// </summary>
		/// <param name="eid">The entity to remove.</param>
		void destroy(uint64_t eid) {
			assert(valid(eid));
			
			// Split the entity into its index and version
			uint32_t idx = dtl::lsw(eid);
			uint32_t ver = dtl::msw(eid);
			
			// Ensure the entity can still be reused
			assert(ver < 0xFFFFFFFF);

			// Revise the entity & make it reusable
			++m_versions[idx];
			m_free_list.push(idx);

			// Destroy all of the entity's components
			for (size_t type_id = 0;
				type_id != m_buffer_metadata.size(); ++type_id) 
			{
				if (idx < m_buffer_metadata[type_id].size()
					&& m_buffer_metadata[type_id].test(idx)) 
				{
					m_buffer_metadata[type_id].reset(idx);
					m_buffers[type_id]->destroy(idx);
				}
			}

			// Remove the entity from all groups it was a part of.
			// No need to check if the group has the entity to begin with, 
			// trying to erase a non existent element of a sparse set is a no op.
			for (auto& [_, group] : m_groups)
				group.erase(eid);
		}

		template <typename InIt>
		void destroy(InIt first, InIt last) {
			for (; first != last; ++first)
				destroy(*first);
		}

		template <typename... Ts>
		bool has(uint64_t eid) const {
			static_assert(sizeof...(Ts) > 0);

			assert(valid(eid));
			if constexpr (sizeof...(Ts) == 1) {
				uint32_t idx = dtl::lsw(eid);
				size_t type_id = dtl::type_index_v<Ts>;

				// Ensure that we're dealing with a component which has
				// been assigned to an entity before.
				assert(type_id < m_buffer_metadata.size()
					&& "Unknown component type!");

				return idx < m_buffer_metadata[type_id].size()
					&& m_buffer_metadata[type_id].test(idx);
			}
			else
				return (has<Ts>(eid) && ...);
		}

		template <typename... Ts>
		void remove(uint64_t eid) {
			assert(has<Ts...>(eid));

			if constexpr (sizeof...(Ts) == 1) {
				uint32_t idx = dtl::lsw(eid);
				size_t type_id = dtl::type_index_v<Ts>;

				// Remove component metadata for the entity.
				m_buffer_metadata[type_id].reset(idx);

				// Destroy the component.
				m_buffers[type_id]->destroy(idx);

				// Remove the entity from all groups which require
				// the recently removed component type
				for (auto& [filter, group] : m_groups)
					if (type_id < filter.size() && filter.test(type_id))
						group.erase(eid);
			}
			else
				(remove<Ts>(eid), ...);
		}

		template <typename T, typename... Args>
		T& assign(uint64_t eid, Args&&... args) {
			using std::make_unique;
			using std::is_constructible_v;

			static_assert(is_constructible_v<T, Args...>);

			assert(!has<T>(eid));

			uint32_t idx = dtl::lsw(eid);
			size_t type_id = dtl::type_index_v<T>;

			// Ensure there exists buffer metadata for the component.
			if (type_id >= m_buffer_metadata.size())
				m_buffer_metadata.resize(type_id + 1);

			// Ensure there exists component metadata for the entity.
			if (idx >= m_buffer_metadata[type_id].size())
				m_buffer_metadata[type_id].resize(idx + 1);

			// Logically add the component to the entity.
			m_buffer_metadata[type_id].set(idx);

			// Ensure there exists a data buffer for the component.
			if (!m_buffers[type_id])
				m_buffers[type_id] = make_unique<dtl::typed_buffer<T>>();

			// Ensure there physically exists memory for the new component
			m_buffers[type_id]->accommodate(idx);

			// Construct the component in memory provided by its buffer
			T* data = static_cast<T*>(m_buffers[type_id]->data(idx));
			::new (data) T(std::forward<Args>(args)...);

			// Add the entity to newly applicable groups.
			// Each time an entity is assigned a new component, it must
			// be added to any existing group which shares a common set
			// of components. This is to ensure that the applicable views
			// automatically pick up the entity.
			for (auto& [filter, group] : m_groups) {
				// Skip groups which already contain this entity.
				if (group.contains(eid))
					continue;
				// Skip groups which do not include the new component.
				if (type_id >= filter.size() || !filter.test(type_id))
					continue;

				// In order to check if an entity belongs to a group
				// We must ensure that for all active bits in the group's 
				// bitset, there exists an active component associated with
				// the entity we're trying to add.
				bool has_all = true;
				for (size_t i = filter.find_first();
					i < filter.size(); i = filter.find_next(i)) 
				{
					if (idx >= m_buffer_metadata[i].size()
						|| !m_buffer_metadata[i].test(idx)) 
					{
						has_all = false;
						break;
					}
				}
				if (has_all)
					group.insert(eid);
			}

			return *data;
		}

		template <typename... Ts>
		decltype(auto) get(uint64_t eid) {
			if constexpr (sizeof...(Ts) == 1) {
				using std::as_const;
				return (const_cast<Ts&>(as_const(*this).template get<Ts>(eid)), ...);
			}
			else {
				using std::forward_as_tuple;
				return forward_as_tuple(
					get<Ts>(eid)...
				);
			}
		}

		template <typename... Ts>
		decltype(auto) get(uint64_t eid) const {
			assert(has<Ts...>(eid));
			if constexpr (sizeof...(Ts) == 1) {
				uint32_t idx = dtl::lsw(eid);
				size_t type_id = (dtl::type_index_v<Ts>, ...);
				return (*static_cast<Ts*>(
							m_buffers[type_id]->data(idx)), ...);
			}
			else {
				using std::forward_as_tuple;
				return forward_as_tuple(
					get<Ts>(eid)...
				);
			}
		}

		size_t size() const {
			return static_cast<size_t>(m_versions.size())
				- static_cast<size_t>(m_free_list.size());
		}

		bool empty() const noexcept {
			return size() == 0;
		}

		template <typename... Ts>
		ecfw::view<Ts...> select();

		template <typename... Ts>
		ecfw::view<Ts...> select() const;

	private:

		template <typename... Ts>
		const dtl::sparse_set& group_by() const {
			using std::max;
			using std::initializer_list;
			
			// Find the largest type id. This will the size of the filter.
			auto type_ids = { dtl::type_index_v<Ts>... };
			size_t largest_type_id = max(type_ids);

			// Ensure we're not working with any unknown components.
			// Type indices are created in sequential order upon discovery by 
			// the system. Any known component is one that has been assigned
			// to an entity is some way. Therefore, any unknown component
			// will not have buffer metadata or a data buffer associated with it
			// and cannot be viewed.
			assert(largest_type_id < m_buffer_metadata.size());

			// Build the filter in order to find an existing group or to create one.
			dynamic_bitset<> filter(largest_type_id);
			for (auto type_id : type_ids)
				filter.set(type_id);

			// Check if there is a cached group.
			auto it = m_groups.find(filter);
			if (it != m_groups.end())
				return it->second;

			// Build the initial group of entities.
			dtl::sparse_set group{};
			uint32_t size = static_cast<uint32_t>(m_versions.size());
			for (size_t idx = 0; idx != size; ++idx) {
				bool has_all = true;
				for (auto type_id : type_ids) {
					if (idx >= m_buffer_metadata[type_id].size() 
						|| !m_buffer_metadata[type_id].test(idx)) 
					{
						has_all = false;
						break;
					}
				}
				if (has_all)
					group.insert(dtl::concat(m_versions[idx], idx));
			}
			it = m_groups.emplace_hint(it, filter, group);
			return it->second;
		}

		// Collection of pointers to entity versions. These pointers 
		// indicate entities that can be reused when making new entities.
		std::stack<uint32_t, std::vector<uint32_t>> m_free_list{};

		// Collection of entity versions, representing their current states.
		std::vector<uint32_t> m_versions{};

		// Component metadata; one bitset for each component type.
		// Space is only allocated in each bitset upon component assignment.
		std::vector<dynamic_bitset<>> m_buffer_metadata{};

		// Component data. Data is only allocated when assigned.
		std::vector<std::unique_ptr<dtl::base_buffer>> m_buffers{};

		// Filtered groups of entities. Each filter represents a common
		// set of components owned by each of the group's entities.
		mutable std::map<dynamic_bitset<>, dtl::sparse_set> m_groups{};

	};

} // namespace ecfw