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
#include <ecfw/detail/entity.hpp>
#include <ecfw/detail/sparse_set.hpp>
#include <ecfw/detail/type_index.hpp>
#include <ecfw/detail/type_traits.hpp>
#include <ecfw/detail/type_list.hpp>

namespace ecfw 
{

	/**
	 * @brief Entity manager.
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
		 * @tparam Ts Components types to initialize the entity with.
		 * @return An entity identifier.
		 */
		template <typename... Ts>
		[[nodiscard]] uint64_t create() {
			using boost::hana::unique;
			using boost::hana::equal;

			// Check for any duplicate types
			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list));

			uint64_t entity{};

			// There exists a reusable entity
			if (!m_free_list.empty()) {
				uint32_t idx = m_free_list.top();
				m_free_list.pop();
				uint32_t ver = m_versions[idx];
				entity = dtl::make_entity(ver, idx);
			}
			else {
				// Ensure it is still possible to create new entities
				auto size = m_versions.size();
				assert(size < 0xFFFFFFFF);

				// Any new entity is effectively a new row in the 
				// component table. However, we refrain from actually 
				// allocating space in the component metabuffer and 
				// component data buffers until components are assigned.
				entity = static_cast<uint64_t>(size);
				m_versions.push_back(0);
			}
			if constexpr (sizeof...(Ts) >= 1)
				(assign<Ts>(entity), ...);
			return entity;
		}
		
		/**
		 * @brief Assigns each element in the range [first, first + n)
		 * an entity created by *this.
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
		[[maybe_unused]] OutIt create_n(OutIt out, size_t n) {
			for (size_t i = 0; i < n; ++i)
				*out++ = create<Ts...>();
			return out;
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
			for (; first != last; ++first) {
				*first = create<Ts...>();
			}
		}

		/**
		 * @brief Constructs a new entity as a clone of another.
		 * 
		 * @tparam T First component type to copy for each entity.
		 * @tparam Ts Component types to copy for each entity.
		 * @param original The entity to copy from.
		 * @return An entity identiter.
		 */
		template <
			typename T,
			typename... Ts
		>
		[[nodiscard]] uint64_t clone(uint64_t original) {
			using std::conjunction_v;

			static_assert(
				conjunction_v<dtl::is_copyable<T>, dtl::is_copyable<Ts>...>);

			uint64_t entity = create();
			(assign<T>(entity, get<T>(original)), 
				..., assign<Ts>(entity, get<Ts>(original)));
			return entity;
		}

		/**
		 * @brief Assigns each element in the range [first, first + n) a 
		 * clone created by *this from original.
		 * 
		 * @tparam T First component type to copy for each entity.
		 * @tparam Ts Component types to copy for each entity.
		 * @tparam OutIt An output iterator type.
		 * @param original The entity to copy from.
		 * @param out An output iterator type.
		 * @param n The number of entities to construct.
		 */
		template <
			typename T,
			typename... Ts, 
			typename OutIt, 
			typename = std::enable_if_t<dtl::is_iterator_v<OutIt>>
		>
		[[maybe_unused]] OutIt clone_n(uint64_t original, OutIt out, size_t n) {
			for (size_t i = 0; i < n; ++i)
				*out++ = clone<T, Ts...>(original);
			return out;
		}

		/**
		 * @brief Assigns each element in the range [first, last) a
		 * clone created by *this from original
		 * 
		 * @tparam T First component type to copy for each entity.
		 * @tparam Ts Other component types to copy for each entity.
		 * @tparam FwdIt Forward iterator type.
		 * @param first Beginning of the range of elements to generate.
		 * @param last One-past the end of the range of elements to generate.
		 */
		template <
			typename T,
			typename... Ts,
			typename FwdIt,
			typename = std::enable_if_t<dtl::is_iterator_v<FwdIt>>
		> 
		void clone(uint64_t original, FwdIt first, FwdIt last) {
			for (; first != last; ++first)
				*first = clone<T, Ts...>();
		}

		/**
		 * @brief Checks if an entity belongs to *this.
		 * 
		 * @param eid The entity to check.
		 * @return true If the entity belongs to *this.
		 * @return false If the entity does not belong to *this.
		 */
		[[nodiscard]] bool valid(uint64_t eid) const {
			auto idx = dtl::index(eid);
			auto ver = dtl::version(eid);
			return idx < m_versions.size()
				&& m_versions[idx] == ver;
		}

		/**
		 * @brief Checks if a collection of entities belong to *this.
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
			using std::all_of;
			return all_of(first, last, [this](auto e) {
				return valid(e); 
			});
		}

		/**
		 * @brief Destroys an entity.
		 * 
		 * @param eid The entity to destroy.
		 */
		void destroy(uint64_t eid) {
			assert(valid(eid));
			
			// Split the entity into its index and version
			auto idx = dtl::index(eid);
			[[maybe_unused]] auto ver = dtl::version(eid);
			
			// Ensure the entity can still be reused
			assert(ver < 0xFFFFFFFF);

			// Revise the entity & make it reusable
			++m_versions[idx];
			m_free_list.push(idx);

			// Destroy all of the entity's components
			for (auto& metabuffer : m_metabuffers) {
				if (idx < metabuffer.size() && metabuffer.test(idx))
					metabuffer.reset(idx);
			}

			// Remove the entity from all groups it's a part of.
			// No need to check if the group has the entity to
			// begin with, trying to erase a non existent element
			// of a sparse set is a no op.
			for (auto& [_, group] : m_groups)
				group.erase(eid);
		}

		/**
		 * @brief Destroys a collection of entities.
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
		 * @brief Checks if a given entity has all of the 
		 * given component types.
		 * 
		 * @tparam Ts Component types to check.
		 * @param eid The entity to check for.
		 * @return true If the given entity has all of the 
		 * given component types.
		 * @return false If the given entity does not have
		 * all of the given component types.
		 */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		[[nodiscard]] bool has(uint64_t eid) const {
			if (!valid(eid))
				return false;
			if constexpr (sizeof...(Ts) == 1) {
				auto idx = dtl::index(eid);
				auto tid = (dtl::type_index_v<Ts>, ...);
				return tid < m_metabuffers.size()
					&& idx < m_metabuffers[tid].size()
					&& m_metabuffers[tid].test(idx);
			}
			else {
				return (has<Ts>(eid) && ...);
			}
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
			using std::conjunction_v;
			using boost::hana::equal;
			using boost::hana::unique;

			assert(valid(eid) && "The entity does not belong to *this.");
			assert(has<Ts...>(eid) 
				&& "The entity does have all of the components given.");

			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(
				equal(unique(type_list), type_list),
				"Duplicate types are not allowed.");

			if constexpr (sizeof...(Ts) == 1) {
				auto idx = dtl::index(eid);
				auto tid = (dtl::type_index_v<Ts>, ...);

				// Remove component metabuffer for the entity.
				m_metabuffers[tid].reset(idx);

				// Remove the entity from all groups which require
				// the recently removed component type. We do not 
				// need to check if the group has the entity because
				// it'll check if it contains any given key before
				// doing an erase.
				for (auto& [filter, group] : m_groups)
					if (tid < filter.size() && filter.test(tid))
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
		 * @param first Beginning of the range of entities to remove 
		 * components from.
		 * @param last One-past the end of the range of entities to 
		 * remove components from.
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
			using std::forward;
			using std::is_default_constructible_v;

			static_assert(is_default_constructible_v<T>);

			static_assert(dtl::is_movable_v<T>);

			assert(valid(eid) && "The entity does not belong to *this.");
			assert(!has<T>(eid) 
				&& "*this does not associate the component with the entity.");

			accommodate<T>();

			auto idx = dtl::index(eid);
			auto tid = dtl::type_index_v<T>;

			// Ensure there exists component metabuffer for the entity.
			if (idx >= m_metabuffers[tid].size())
				m_metabuffers[tid].resize(idx + 1);

			// Logically add the component to the entity.
			m_metabuffers[tid].set(idx);

			auto& component_buffer = buffer<T>();

			// Ensure there physically exists memory for the new component
			if (idx >= component_buffer.size())
				component_buffer.resize(idx + 1);

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
				if (tid >= filter.size() || !filter.test(tid))
					continue;

				// In order to check if an entity belongs to a group
				// We must ensure that for all active bits in the group's 
				// bitset, there exists an active component associated with
				// the entity we're trying to add.
				bool has_all = true;
				size_t i = filter.find_first();
				for (; i < filter.size(); i = filter.find_next(i)) {
					const auto& metabuffer = m_metabuffers[i];
					if (idx >= metabuffer.size() || !metabuffer.test(idx)) {
						has_all = false;
						break;
					}
				}
				if (has_all)
					group.insert(eid);
			}
			
			// Construct and return the component.
			return construct(component_buffer, idx, forward<Args>(args)...);
		}

		/**
		 * @brief Assigns components to a collection of entities.
		 * 
		 * @tparam Ts Component types to assign to each entity.
		 * @tparam InIt Input iterator type. 
		 * @param first Beginning of the range of entities to assign 
		 * components to.
		 * @param last One-past the end of the range of entities to 
		 * assign components to.
		 */
		template <
			typename... Ts,
			typename InIt,
			typename = std::enable_if_t<dtl::is_iterator_v<InIt>>
		> void assign(InIt first, InIt last) {
			using boost::hana::equal;
			using boost::hana::unique;

			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list));

			for (; first != last; ++first)
				(assign<Ts>(*first), ...);
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
			using std::forward;

			assert(valid(eid) && "The entity does not belong to *this.");

			if (!has<T>(eid)) {
				return assign<T>(eid, forward<Args>(args)...);
			}
			else {
				auto idx = dtl::index(eid);
				return construct(buffer<T>(), idx, forward<Args>(args)...);
			}
		}

		/**
		 * @brief Returns an entity's components
		 * 
		 * @tparam Ts Component types to fetch.
		 * @param eid The entity to fetch for.
		 * @return Reference to a component or a tuple of references.
		 */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		[[nodiscard]] decltype(auto) get(uint64_t eid) {
			assert(has<Ts...>(eid)
				&& "The entity does not have all of the components given.");
			if constexpr (sizeof...(Ts) == 1) {
				auto idx = dtl::index(eid);
				return (buffer<Ts>()[idx], ...);
			}
			else {
				using std::forward_as_tuple;				
				return forward_as_tuple(get<Ts>(eid)...);
			}
		}

		/*! @copydoc get */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		[[nodiscard]] decltype(auto) get(uint64_t eid) const {
			using std::is_const;
			using std::conjunction_v;

			static_assert(conjunction_v<is_const<Ts>...>);

			assert(has<Ts...>(eid)
				&& "The entity does not have all of the components given.");
			if constexpr (sizeof...(Ts) == 1) {
				auto idx = dtl::index(eid);
				return (buffer<Ts>()[idx], ...);
			}
			else {
				using std::forward_as_tuple;
				return forward_as_tuple(get<Ts>(eid)...);
			}
		}
		
		/**
		 * @brief Returns the number of created entities.
		 * 
		 * @return The number of created entities.
		 */
		[[nodiscard]] size_t num_entities() const noexcept {
			return static_cast<size_t>(m_versions.size());
		}

		/**
		 * @brief Returns the number of entities that have not been destroyed.
		 * 
		 * @return The number of entities that have not been destroyed.
		 */
		[[nodiscard]] size_t num_alive() const noexcept {
			assert(num_entities() >= num_reusable());
			return num_entities() - num_reusable();
		}

		/**
		 * @brief Returns the number of entities that can use reused.
		 * 
		 * @return The number entities that can be reused.
		 */
		[[nodiscard]] size_t num_reusable() const noexcept {
			return static_cast<size_t>(m_free_list.size());
		}

		/**
		 * @brief Returns the number of entities which have the given 
		 * components.
		 * 
		 * @tparam Ts Components types the entities must have.
		 * @return The number of entities which have the components.
		 */
		template <typename T, typename... Ts>
		[[nodiscard]] size_t count() const { // rename to count
			using std::count_if;
			auto unary_predicate = [i = 0,this](auto v) mutable { 
				auto entity = dtl::make_entity(v, i++);
				return has<T, Ts...>(entity); 
			};
			auto ret = count_if(
				m_versions.begin(), m_versions.end(), unary_predicate);
			return static_cast<size_t>(ret);
		}

		/**
		 * @brief Returns the maximum number of elements the component vector
		 * for a given component type is able to hold.
		 * 
		 * @tparam T Component type to check.
		 * @return The maximum number of elements.
		 */
		template <typename T>
		[[nodiscard]] size_t max_size() const  {
			return static_cast<size_t>(buffer<T>().max_size());
		}

		/**
		 * @brief Returns the number of elements in the component vector for 
		 * the given type.
		 * 
		 * @tparam T Component type of the component vector.
		 * @return The number of elements in the compnent vector.
		 */
		template <typename T>
		[[nodiscard]] size_t size() const {
			return static_cast<size_t>(buffer<T>().size());
		}

		/**
		 * @brief Checks if the component vector for a given type is empty.
		 * 
		 * @tparam T Component type of the component vector.
		 * @return true if the component vector is empty.
		 * @return false if the component vector is not empty.
		 */
		template <typename T>
		[[nodiscard]] bool empty() const {
			return buffer<T>().empty();
		}

		/**
		 * @brief Returns the capacity of the component vector for a given type.
		 * 
		 * @tparam T Component type of the component vector.
		 * @return The capacity of the component vector.
		 */
		template <typename T>
		[[nodiscard]] size_t capacity() const {
			return static_cast<size_t>(buffer<T>().capacity());
		}

		/**
		 * @brief Requests the removal of unused capacity from the component
		 * vectors associated with a given set of component types.
		 * 
		 * @tparam Ts Component types of the component vectors.
		 */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		void shrink_to_fit() {
			if constexpr (sizeof...(Ts) == 1)
				(buffer<Ts>().shrink_to_fit(), ...);
			else 
				(shrink_to_fit<Ts>(), ...);
		}

		/**
		 * @brief Reserves storage space for the given component types if any.
		 * Otherwise, reserves storage space for entities.
		 * 
		 * @tparam Ts Component types to reserve space for.
		 * @param n The number of components to allocated space for.
		 */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		void reserve(size_t n) {
			if constexpr (sizeof...(Ts) == 1) {
				using std::make_any;
				using std::any_cast;
				using std::make_unique;

				accommodate<Ts...>();

				auto tid = (dtl::type_index_v<Ts>, ...);
				
				// Ensure there exists component metabuffer up to index n.
				m_metabuffers[tid].reserve(n);
				
				// Ensure there physically exists memory for n components.
				(buffer<Ts>().reserve(n), ...);
			}
			else {
				(reserve<Ts>(n), ...);
			}
		}

		/**
		 * @brief Returns a view of a given set of components.
		 * 
		 * @tparam Ts Component types to be viewed.
		 * @return An instance of ecfw::view.
		 */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		[[nodiscard]] ecfw::view<Ts...> view() {
			using boost::hana::equal;
			using boost::hana::unique;

			// Check for duplicate types
			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list));

			accommodate<Ts...>();

			return ecfw::view<Ts...>{ group_by<Ts...>(), buffer<Ts>()... };
		}

		/*! @copydoc view */
		template <
			typename... Ts,
			typename = std::enable_if_t<sizeof...(Ts) >= 1>
		>
		[[nodiscard]] ecfw::view<Ts...> view() const {
			using std::is_const;
			using std::conjunction_v;
			using boost::hana::equal;
			using boost::hana::unique;
			
			// Check for duplicate types
			constexpr auto type_list = dtl::type_list_v<Ts...>;
			static_assert(equal(unique(type_list), type_list));
			
			// Check that all requested types are const
			static_assert(conjunction_v<is_const<Ts>...>);

			accommodate<Ts...>();

			return ecfw::view<Ts...>{ group_by<Ts...>(), buffer<Ts>()... };
		}

	private:

		template <typename T>
		[[nodiscard]] const std::vector<std::decay_t<T>>& buffer() const {
			using std::decay_t;
			using std::any_cast;

			auto tid = dtl::type_index_v<T>;
			assert(tid < m_buffers.size());
			return any_cast<const std::vector<decay_t<T>>&>(m_buffers[tid]);
		}

		template <typename T>
		[[nodiscard]] std::vector<std::decay_t<T>>& buffer() {
			using std::decay_t;
			using std::any_cast;
			
			auto tid = dtl::type_index_v<T>;
			assert(tid < m_buffers.size());
			return any_cast<std::vector<decay_t<T>>&>(m_buffers[tid]);
		}

		template <typename T, typename... Args>
		[[nodiscard]] T& construct(
			std::vector<T>& buffer, uint32_t idx, Args&&... args) {
				
			using std::forward;
			using std::is_aggregate_v;
			using std::is_constructible_v;

			if constexpr (is_aggregate_v<T>)
				buffer[idx] = T{forward<Args>(args)...};
			else {
				static_assert(is_constructible_v<T, Args...>);
				buffer[idx] = T(forward<Args>(args)...);
			}
			return buffer[idx];
		}

		template <typename... Ts>
		void accommodate() const {
			if constexpr (sizeof...(Ts) == 1) {
				using std::vector;
				using std::decay_t;
				using std::make_any;

				auto tid = (dtl::type_index_v<Ts>, ...);

				if (tid >= m_metabuffers.size())
					m_metabuffers.resize(tid + 1);
				
				if (tid >= m_buffers.size())
					m_buffers.resize(tid + 1);
				
				if (!m_buffers[tid].has_value())
					m_buffers[tid] = 
						(make_any<vector<decay_t<Ts>>>(), ...);
			}
			else
				(accommodate<Ts>(), ...);
		}

		using group_map_type = 
			std::unordered_map<boost::dynamic_bitset<>, dtl::sparse_set>;
		using group_key_type = typename group_map_type::key_type;
		using group_mapped_type = typename group_map_type::mapped_type;

		template <typename... Ts>
		[[nodiscard]] const dtl::sparse_set& group_by() const {
			using std::max;
			using std::move;
			using std::for_each;
			using std::initializer_list;
			
			// Find the largest type id. Size of the group id is +1.
			auto tids = { dtl::type_index_v<Ts>... };
			size_t largest_tid = max(tids);

			// Ensure we're not working with any unknown components.
			// Type indices are created in sequential order upon 
			// discovery by any world either by assignment, view
			// creation, or storage reservation.
			assert(largest_tid < m_metabuffers.size());

			// Build the filter in order to find an existing 
			// group or to create one.
			group_key_type filter(largest_tid + 1);
			for (auto tid : tids)
				filter.set(tid);

			// Check if there exists a group identified by our filter.
			// If so, return it to the caller.
			auto it = m_groups.find(filter);
			if (it != m_groups.end())
				return it->second;

			// Build the initial group of entities.
			group_mapped_type group{};
			auto unary_function = [i = 0,this,&group](auto v) mutable {
				auto entity = dtl::make_entity(v, i++);
				if (has<Ts...>(entity))
					group.insert(entity);
			};
			for_each(m_versions.begin(), m_versions.end(), unary_function);
			it = m_groups.emplace_hint(it, filter, move(group));
			return it->second;
		}

		// Collection of pointers to entity versions. These pointers 
		// indicate entities that can be reused when making new entities.
		std::stack<uint32_t, std::vector<uint32_t>> m_free_list{};

		// Collection of entity versions, where a version indicates how
		// many times an entity has been destroyed/reused.
		std::vector<uint32_t> m_versions{};

		// Component metabuffer; one bitset for each component type. 
		// A bitset is only allocated for a component when it's first 
		// assigned to an entity, when a view is created, or when space 
		// is reserved for a component1. W.r.t assignment, only enough 
		// space within the bitset is allocated to accommodate at most
		// the entity involved in the assignment. 
		mutable std::vector<dtl::metabuffer_type> m_metabuffers{};

		// Component data. Space is allocated similarly to the 
		// metabuffers collection. We use a std::any to hide
		// the buffer type until we're in a context where we can
		// deduce the type of the buffer.
		mutable std::vector<std::any> m_buffers{};

		// Filtered groups of entities. Each filter represents a common
		// set of components each entity of the group possesses.
		mutable group_map_type m_groups{};

	};

} // namespace ecfw