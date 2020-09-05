#pragma once

#include <any>	
#include <tuple>
#include <stack>
#include <vector>
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <boost/hana.hpp>
#include <boost/dynamic_bitset.hpp>
#include <ecfw/core/view.hpp>
#include <ecfw/detail/buffer.hpp>
#include <ecfw/detail/dword.hpp>
#include <ecfw/detail/sparse_set.hpp>
#include <ecfw/detail/type_index.hpp>
#include <ecfw/detail/type_traits.hpp>
#include <ecfw/detail/type_list.hpp>

namespace ecfw 
{

	/**
	 * @brief Heterogeneous structure of arrays.
	 * 
	 */
	class world final {
	public:

		/**
		 * @brief Default constructor.
		 * 
		 */
		world() = default;

		/**
		 * @brief Default move constructor.
		 * 
		 */
		world(world&&) = default;

		/**
		 * @brief Default move assignment operator.
		 * 
		 * @return *this.
		 */
		world& operator=(world&&) = default;

		/**
		 * @brief Default destructor.
		 * 
		 */
		~world() = default;

		/**
		 * @brief Constructs a new entity.
		 * 
		 * @pre 
		 * Each given component type must satisfy:
		 * std::is_default_constructible_v<T> == true
		 * 
		 * @tparam Ts Components types to initialize the entity with.
		 * @return An entity identifier.
		 */
		template <typename... Ts>
		[[nodiscard]] uint64_t create() {
			using std::conjunction_v;
			using std::is_default_constructible;
			using boost::hana::unique;
			using boost::hana::equal;

			// Ensure that each given component is default constructible
			static_assert(conjunction_v<is_default_constructible<Ts>...>, 
				"Assigning components through world::create requires default constructible types");

			// Check for any duplicate types
			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list), 
				"Duplicate types are not allowed!");

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
				auto size = m_versions.size();
				assert(size < 0xFFFFFFFF);

				// Any new entity is effectively a new row in the component table.
				// However, we refrain from actually allocating space in the component
				// metadata and component data buffers until components are assigned.
				entity = static_cast<uint64_t>(size);
				m_versions.push_back(0);
			}
			if constexpr (sizeof...(Ts) > 0)
				(assign<Ts>(entity), ...);
			return entity;
		}

		/**
		 * @brief Assigns each element in the range [first, first + n) an
		 * entity created by *this.
		 * 
		 * @pre 
		 * Each given component type must satisfy:
		 * std::is_default_constructible_v<T> == true.
		 * 
		 * @tparam Ts Component types to initialize each entity with.
		 * @tparam OutIt An output iterator type.
		 * @param out An output iterator.
		 * @param n The number of entities to construct.
		 */
		template <
			typename... Ts, 
			typename OutIt, 
			typename = std::enable_if_t<dtl::is_iterator_v<OutIt>>
		>
		void create_n(OutIt out, size_t n) {
			std::generate_n(out, n, [this]() mutable {
				return create<Ts...>(); 
			});
		}

		/**
		 * @brief Assigns each element in the range [first, last) an 
		 * entity created by *this.
		 * 
		 * @tparam Ts Component types to initialze each entity with.
		 * @tparam FwdIt Forward iterator type.
		 * @param first Beginning of the range of elements to generate.
		 * @param last One-past the end of the range of elements to generate.
		 */
		template <
			typename... Ts,
			typename FwdIt,
			typename = std::enable_if_t<dtl::is_iterator_v<FwdIt>>
		>
		void create(FwdIt first, FwdIt last) {
			std::generate(first, last, [this]() mutable {
				return create<Ts...>();
			});
		}

		/**
		 * @brief Constructs a new entity as a clone of another.
		 * 
		 * @pre
		 * Each given component must satisfy: 
		 * std::is_copy_constructible_v<T> == true
		 * 
		 * @tparam Ts Component types to copy.
		 * @param original The entity to copy from.
		 * @return An entity identiter.
		 */
		template <typename... Ts>
		[[nodiscard]] uint64_t clone(uint64_t original) {
			using std::conjunction_v;
			using std::is_copy_constructible;
			static_assert(conjunction_v<is_copy_constructible<Ts>...>, 
				"Cloning components requires copy constructible component types.");
			uint64_t entity = create();
			(assign<Ts>(entity, get<Ts>(original)), ...);
			return entity;
		}

		/**
		 * @brief Assigns each element in the range [first, first + n) a 
		 * clone created by *this from original.
		 * 
		 * @pre
		 * Each given component must satisfy: 
		 * std::is_copy_constructible_v<T> == true
		 * 
		 * @tparam Ts Component types to copy for each entity.
		 * @tparam OutIt An output iterator type.
		 * @param original The entity to copy from.
		 * @param out An output iterator type.
		 * @param n The number of entities to construct.
		 */
		template <
			typename... Ts, 
			typename OutIt, 
			typename = std::enable_if_t<dtl::is_iterator_v<OutIt>>
		>
		void clone_n(uint64_t original, OutIt out, size_t n) {
			std::generate_n(out, n, [this, original]() mutable {
				 return clone<Ts...>(original); 
			});
		}

		/**
		 * @brief Assigns each element in the range [first, last) a
		 * clone created by *this from original
		 * 
		 * @tparam Ts Component types to copy for each entity.
		 * @tparam FwdIt Forward iterator type.
		 * @param first Beginning of the range of elements to generate.
		 * @param last One-past the end of the range of elements to generate.
		 */
		template <
			typename... Ts,
			typename FwdIt,
			typename = std::enable_if_t<dtl::is_iterator_v<FwdIt>>
		> 
		void clone(uint64_t original, FwdIt first, FwdIt last) {
			std::generate(first, last, [this, original]() mutable {
				return clone<Ts...>(original);
			});
		}

		/**
		 * @brief Checks if an entity belongs to *this.
		 * 
		 * @param eid The entity to check.
		 * @return true If the entity belongs to *this.
		 * @return false If the entity does not belong to *this.
		 */
		[[nodiscard]] bool valid(uint64_t eid) const {
			auto idx = dtl::lsw(eid);
			auto ver = dtl::msw(eid);
			return idx < m_versions.size()
				&& m_versions[idx] == ver;
		}

		/**
		 * @brief Checks if a collection of entities belong to *this.
		 * 
		 * This member function is equivalent to the following
		 * snippet:
		 * @code
		 * for (auto entity : entities)
		 *     world.valid(entity);
		 * @endcode
		 * 
		 * @tparam InIt An input iterator type.
		 * @param first The beginning of the sequence to verify
		 * @param last One-past the end of the sequence
		 * @return true If all elements of the sequence belong to *this
		 * @return false If there exists one or more elements which do not
		 * belong to this
		 */
		template <
			typename InIt, 
			typename = std::enable_if_t<dtl::is_iterator_v<InIt>>
		>
		[[nodiscard]] bool valid(InIt first, InIt last) const {
			return std::all_of(first, last, 
				[this](auto e) { 
					return valid(e); 
				}
			);
		}

		/**
		 * @brief Destroys an entity.
		 * 
		 * @warning 
		 * Upon destruction, all references to the entity's
		 * components become invalid.
		 * 
		 * @param eid The entity to destroy.
		 */
		void destroy(uint64_t eid) {
			assert(valid(eid));
			
			// Split the entity into its index and version
			auto idx = dtl::lsw(eid);
			[[maybe_unused]] auto ver = dtl::msw(eid);
			
			// Ensure the entity can still be reused
			assert(ver < 0xFFFFFFFF);

			// Revise the entity & make it reusable
			++m_versions[idx];
			m_free_list.push(idx);

			// Destroy all of the entity's components
			for (auto type_id = 0; type_id != m_metadatas.size(); ++type_id) {
				auto& metadata = m_metadatas[type_id];
				if (idx < metadata.size() && metadata.test(idx)) 
					m_metadatas[type_id].reset(idx);
			}

			// Remove the entity from all groups it's a part of.
			// No need to check if the group has the entity to begin with, 
			// trying to erase a non existent element of a sparse set is a no op.
			for (auto& [_, group] : m_groups)
				group.erase(eid);
		}

		/**
		 * @brief Destroys a collection of entities.
		 * 
		 * @warning 
		 * Upon destruction, all references to the components become invalid for 
		 * each entity destroyed.
		 * 
		 * @tparam InIt An input iterator type.
		 * @param first An iterator to the beginning of a range.
		 * @param last An iterator to one-past the end of a range.
		 */
		template <
			typename InIt, 
			typename = std::enable_if_t<dtl::is_iterator_v<InIt>>
		>
		void destroy(InIt first, InIt last) {
			for (; first != last; ++first)
				destroy(*first);
		}
		
		/**
		 * @brief Checks if an entity owns a given set of components.
		 * 
		 * @tparam Ts Component types to check for.
		 * @param eid The entity to check. 
		 * @return true If the entity has each component.
		 * @return false If the entity does not have each component.
		 */
		template <
			typename... Ts, 
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		[[nodiscard]] bool has(uint64_t eid) const {
			using boost::hana::unique;
			using boost::hana::equal;

			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list),
				"Duplicate types are not allowed!");

			assert(valid(eid));
			if constexpr (sizeof...(Ts) == 1) {
				auto idx = dtl::lsw(eid);
				auto type_id = (dtl::type_index_v<Ts>, ...);

				return type_id < m_metadatas.size()
					&& idx < m_metadatas[type_id].size()
					&& m_metadatas[type_id].test(idx);
			}
			else
				return (has<Ts>(eid) && ...);
		}

		/**
		 * @brief Destroys components associated with an entity.
		 * 
		 * @tparam Ts Component types to remove.
		 * @param eid The entity to remove from.
		 */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		void remove(uint64_t eid) {
			assert(has<Ts...>(eid));

			if constexpr (sizeof...(Ts) == 1) {
				auto idx = dtl::lsw(eid);
				auto type_id = (dtl::type_index_v<Ts>, ...);

				// Remove component metadata for the entity.
				m_metadatas[type_id].reset(idx);

				// Remove the entity from all groups which require
				// the recently removed component type
				for (auto& [filter, group] : m_groups)
					if (type_id < filter.size() && filter.test(type_id))
						group.erase(eid);
			}
			else
				(remove<Ts>(eid), ...);
		}

		/**
		 * @brief Removes components from a collection of entities.
		 * 
		 * @tparam Ts Component types to remove.
		 * @tparam InIt Input iterator type.
		 * @param first Beginning of the range of entities to remove components from.
		 * @param last One-past the end of the range of entities to remove components from.
		 */
		template <
			typename... Ts,
			typename InIt,
			typename = std::enable_if_t<dtl::is_iterator_v<InIt>>
		> void remove(InIt first, InIt last) {
			for (; first != last; ++first)
				remove<Ts...>(*first);
		}

		/**
		 * @brief Constructs a new component associated with an entity.
		 * 
		 * @pre
		 * The given component type must satisfy the following:
		 * std::is_constructible_v<T, Args...> == true.
		 * 
		 * @tparam T Component type.
		 * @tparam Args Component constructor parameter types. 
		 * @param eid The entity the component will be associated with.
		 * @param args Component constructor parameters.
		 * @return Reference to the constructed component.
		 */
		template <
			typename T, 
			typename... Args
		>
		[[maybe_unused]] T& assign(uint64_t eid, Args&&... args) {
			using std::vector;
			using std::forward;
			using std::make_any;
			using std::any_cast;
			using std::make_unique;
			using std::conjunction_v;
			using std::is_aggregate_v;
			using std::is_constructible_v;
			using std::is_move_assignable;
			using std::is_move_constructible;
			using std::is_default_constructible_v;

			static_assert(is_default_constructible_v<T>,
				"All component types must be default constructible.");

			static_assert(conjunction_v<is_move_constructible<T>, is_move_assignable<T>>,
				"Assigned component types must be at least move constructible/assignable.");

			static_assert(is_constructible_v<T, Args...>, 
				"Cannot construct the component from the given arguments.");

			assert(!has<T>(eid));

			accommodate<T>();

			auto idx = dtl::lsw(eid);
			auto type_id = dtl::type_index_v<T>;

			// Ensure there exists component metadata for the entity.
			if (idx >= m_metadatas[type_id].size())
				m_metadatas[type_id].resize(idx + 1);

			// Logically add the component to the entity.
			m_metadatas[type_id].set(idx);

			auto& buffer = any_cast<vector<T>&>(m_buffers[type_id]);

			// Ensure there physically exists memory for the new component
			if (idx >= buffer.size())
				buffer.resize(idx + 1);

			// Construct the component 
			if constexpr (is_aggregate_v<T>)
				buffer[idx] = T{forward<Args>(args)...};
			else 
				buffer[idx] = T(forward<Args>(args)...);

			// Add the entity to all newly applicable groups.
			// Each time an entity is assigned a new component, it must
			// be added to any existing group which shares a common set
			// of components to ensure that the applicable views
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
				size_t i = filter.find_first();
				for (; i < filter.size(); i = filter.find_next(i)) {
					const auto& metadata = m_metadatas[i];
					if (idx >= metadata.size() || !metadata.test(idx)) {
						has_all = false;
						break;
					}
				}
				if (has_all)
					group.insert(eid);
			}

			return buffer[idx];
		}

		/**
		 * @brief If the entity does not have the given component type,
		 * a new component will be assigned to it. Otherwise, replace
		 * the entity's current component.
		 * 
		 * @tparam T Component type to assign or replace.
		 * @tparam Args Component constructor argument types.
		 * @param eid Candidate entity.
		 * @param args Component constructor argument values.
		 * @return A reference to the newly constructed component.
		 */
		template <
			typename T, 
			typename... Args
		>
		[[maybe_unused]] T& assign_or_replace(uint64_t eid, Args&&... args) {
			using std::vector;
			using std::forward;
			using std::any_cast;
			using std::conjunction_v;
			using std::is_aggregate_v;
			using std::is_constructible_v;
			using std::is_move_assignable;
			using std::is_move_constructible;

			static_assert(conjunction_v<is_move_constructible<T>, is_move_assignable<T>>,
				"Assigned component types must be at least move constructible/assignable.");

			static_assert(is_constructible_v<T, Args...>, 
				"Cannot construct the component from the given arguments.");

			if (!has<T>(eid)) {
				return assign<T>(eid, forward<Args>(args)...);
			}
			else {
				auto idx = dtl::lsw(eid);
				auto type_id = dtl::type_index_v<T>;

				auto& buffer = any_cast<vector<T>&>(m_buffers[type_id]);
				if constexpr (is_aggregate_v<T>)
					buffer[idx] = T{forward<Args>(args)...};
				else 
					buffer[idx] = T(forward<Args>(args)...);
				return buffer[idx];
			}
		}

		/**
		 * @brief Assigns components to a collection of entities.
		 * 
		 * @tparam Ts Component types to assign to each entity.
		 * @tparam InIt Input iterator type. 
		 * @param first Beginning of the range of entities to assign components to.
		 * @param last One-past the end of the range of entities to assign components to.
		 */
		template <
			typename... Ts,
			typename InIt,
			typename = std::enable_if_t<dtl::is_iterator_v<InIt>>
		> void assign(InIt first, InIt last) {
			for (; first != last; ++first)
				(assign<Ts>(*first), ...);
		}

		/**
		 * @brief Returns an entity's components
		 * 
		 * @tparam Ts Component types to fetch.
		 * @param eid The entity to fetch for.
		 * @return Reference to a component or a tuple of references.
		 */
		template <typename... Ts>
		[[nodiscard]] decltype(auto) get(uint64_t eid) {
			if constexpr (sizeof...(Ts) == 1) {
				using std::as_const;

				return (const_cast<Ts&>(as_const(*this). template get<Ts>(eid)), ...);
			}
			else {
				using std::forward_as_tuple;
				
				return forward_as_tuple(
					get<Ts>(eid)...
				);
			}
		}

		/*! @copydoc get */
		template <typename... Ts>
		[[nodiscard]] decltype(auto) get(uint64_t eid) const {
			assert(has<Ts...>(eid));
			if constexpr (sizeof...(Ts) == 1) {
				using std::vector;
				using std::any_cast;
				using std::decay_t;

				auto idx = dtl::lsw(eid);
				auto type_id = (dtl::type_index_v<decay_t<Ts>>, ...);
				const auto& buffer = (any_cast<const vector<decay_t<Ts>>&>(m_buffers[type_id]), ...);
				return buffer[idx];
			}
			else {
				using std::forward_as_tuple;

				return forward_as_tuple(
					get<Ts>(eid)...
				);
			}
		}

		/**
		 * @brief Returns the number of active entities.
		 * 
		 * @tparam Ts Components types the entities must have.
		 * @return The number of active entities.
		 */
		template <typename... Ts>
		[[nodiscard]] size_t size() const {
			if constexpr (sizeof...(Ts) > 0) {
				size_t count = 0;
				uint32_t size = static_cast<uint32_t>(m_versions.size());
				for (uint32_t idx = 0; idx != size; ++idx) {
					uint64_t entity = dtl::concat(m_versions[idx], idx);
					if (has<Ts...>(entity))
						++count;
				}
				return count;
			}
			else {
				return static_cast<size_t>(m_versions.size())
					- static_cast<size_t>(m_free_list.size());
			}
		}

		/**
		 * @brief Checks if *this does not have any active entities.
		 * 
		 * @tparam Ts Component types the entities must have.
		 * @return true if there are no active entities.
		 * @return false if there area active entities.
		 */
		template <typename... Ts>
		[[nodiscard]] bool empty() const {
			return size<Ts...>() == 0;
		}

		/**
		 * @brief Preallocates storage space for the given component types.
		 * 
		 * @tparam Ts Component types to reserve space for.
		 * @param n The number of components to allocated space for.
		 */
		template <typename... Ts>
		void reserve(size_t n) {
			if constexpr (sizeof...(Ts) == 1) {
				using std::vector;
				using std::make_any;
				using std::any_cast;
				using std::make_unique;

				accommodate<Ts...>();

				auto type_id = (dtl::type_index_v<Ts>, ...);
				
				// Ensure there exists component metadata up to index n.
				m_metadatas[type_id].reserve(n);
				
				// Ensure there physically exists memory for n components.
				(any_cast<vector<Ts>&>(m_buffers[type_id]).reserve(n), ...);
			}
			else if constexpr (sizeof...(Ts) > 1){
				(reserve<Ts>(n), ...);
			}
			else 
				m_versions.reserve(n);
		}

		/**
		 * @brief Returns a view of a given set of components.
		 * 
		 * @tparam Ts Component types to be viewed.
		 * @return An instance of ecfw::view.
		 */
		template <typename... Ts>
		[[nodiscard]] ecfw::view<Ts...> view() {
			using std::vector;
			using std::decay_t;
			using std::any_cast;
			using boost::hana::equal;
			using boost::hana::unique;

			// Check for duplicate types
			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list),
				"Cannot construct view with duplicate components.");

			accommodate<std::decay_t<Ts>...>();

			return ecfw::view<Ts...>{
				group_by<decay_t<Ts>...>(),
				any_cast<vector<decay_t<Ts>>&>(m_buffers[dtl::type_index_v<decay_t<Ts>>])...
			};
		}

		/*! @copydoc view */
		template <typename... Ts>
		[[nodiscard]] ecfw::view<Ts...> view() const {
			using std::vector;
			using std::decay_t;
			using std::any_cast;
			using std::is_const;
			using std::conjunction_v;
			using boost::hana::equal;
			using boost::hana::unique;
			
			// Check for duplicate types
			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list),
				"Cannot construct view with duplicate components.");
			
			// Check that all requested types are const
			static_assert(conjunction_v<is_const<Ts>...>,
				"Creating views from a const world requires all requested types are const.");

			accommodate<std::decay_t<Ts>...>();

			return ecfw::view<Ts...>{
				group_by<decay_t<Ts>...>(),
				any_cast<const vector<decay_t<Ts>>&>(m_buffers[dtl::type_index_v<decay_t<Ts>>])...
			};
		}

	private:

		template <typename... Ts>
		void accommodate() const {
			if constexpr (sizeof...(Ts) == 1) {
				using std::vector;
				using std::make_any;

				auto type_id = (dtl::type_index_v<Ts>, ...);

				if (type_id >= m_metadatas.size())
					m_metadatas.resize(type_id + 1);
				
				if (type_id >= m_buffers.size())
					m_buffers.resize(type_id + 1);
				
				if (!m_buffers[type_id].has_value())
					m_buffers[type_id] = (make_any<vector<Ts>>(), ...);
			}
			else
				(accommodate<Ts>(), ...);
		}

		template <typename... Ts>
		[[nodiscard]] const dtl::sparse_set& group_by() const {
			using std::max;
			using std::initializer_list;
			
			// Find the largest type id. Size of the filter is +1.
			auto type_ids = { dtl::type_index_v<Ts>... };
			size_t largest_type_id = max(type_ids);

			// Ensure we're not working with any unknown components.
			// Type indices are created in sequential order upon discovery by 
			// any world either by assignment or view creation.
			assert(largest_type_id < m_metadatas.size());

			// Build the filter in order to find an existing group or to create one.
			boost::dynamic_bitset<> filter(largest_type_id + 1);
			for (auto type_id : type_ids)
				filter.set(type_id);

			// Check if there exists a group identified by our filter.
			// If so, return it to the caller.
			auto it = m_groups.find(filter);
			if (it != m_groups.end())
				return it->second;

			// Build the initial group of entities.
			dtl::sparse_set group{};
			uint32_t size = static_cast<uint32_t>(m_versions.size());
			for (uint32_t idx = 0; idx != size; ++idx) {
				auto entity = dtl::concat(m_versions[idx], idx);
				if (has<Ts...>(entity))
					group.insert(entity);
			}
			it = m_groups.emplace_hint(it, filter, std::move(group));
			return it->second;
		}

		// Collection of pointers to entity versions. These pointers 
		// indicate entities that can be reused when making new entities.
		std::stack<uint32_t, std::vector<uint32_t>> m_free_list{};

		// Collection of entity versions, where a version indicates how
		// many times an entity has been destroyed/reused.
		std::vector<uint32_t> m_versions{};

		// Component metadata; one bitset for each component type. A bitset is only 
		// allocated for a compoentn when it's first assigned to an entity. Moreover,
		// only enough space is allocated within the bitset to accommodate the entity 
		// to which it the component is assigned.
		mutable std::vector<boost::dynamic_bitset<>> m_metadatas{};

		// Component data. space is allocated similarly to the buffer metadata.
		mutable std::vector<std::any> m_buffers{};

		// Filtered groups of entities. Each filter represents a common
		// set of components each of the entities in the group must possess.
		mutable std::unordered_map<boost::dynamic_bitset<>, dtl::sparse_set> m_groups{};

	};

} // namespace ecfw