#pragma once

#include <any>	
#include <tuple>
#include <stack>
#include <vector>
#include <cassert>
#include <concepts>
#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <boost/dynamic_bitset.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <ecfw/view.hpp>
#include <ecfw/detail/buffer.hpp>
#include <ecfw/detail/entity.hpp>
#include <ecfw/detail/sparse_set.hpp>
#include <ecfw/detail/type_index.hpp>
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
            requires (std::default_initializable<Ts> && ...)
        [[nodiscard]] uint64_t create() {
            // Check for duplicate component types
            static_assert(dtl::is_unique(dtl::type_list_v<Ts...>));

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
         * @brief Creates n-many entities.
         * 
         * @tparam T The first component the entity should start with.
         * @tparam Ts The other components the entity should start with.
         * @param n The number of entities to create.
         */
        template <typename T, typename... Ts>
            requires (std::default_initializable<T> 
                && ... && std::default_initializable<Ts>)
        void create(size_t n) {
            for (size_t i = 0; i < n; ++i)
                (void)create<T, Ts...>();
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
        template <typename... Ts, typename OutIt>
            requires (std::default_initializable<Ts> && ...)
                && std::output_iterator<OutIt, uint64_t>
                && std::same_as<std::iter_value_t<OutIt>, uint64_t>
        [[maybe_unused]] OutIt create(OutIt out, size_t n) {
            auto generate_entity = [this](){ return create<Ts...>(); };
            return std::generate_n(out, n, generate_entity);
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
        template <typename... Ts, typename FwdIt>
            requires (std::default_initializable<Ts> && ...)
                && std::forward_iterator<FwdIt>
                && std::same_as<std::iter_value_t<FwdIt>, uint64_t>
        void create(FwdIt first, FwdIt last) {
            auto generate_entity = [this](){ return create<Ts...>(); };
            std::generate(first, last, generate_entity);
        }

        /**
         * @brief Constructs a new entity as a clone of another.
         * 
         * @tparam T First component type to copy for each entity.
         * @tparam Ts Component types to copy for each entity.
         * @param original The entity to copy from.
         * @return An entity identiter.
         */
        template <typename T,typename... Ts>
            requires (std::copyable<T> && ... && std::copyable<Ts>)
        [[nodiscard]] uint64_t clone(uint64_t original) {
            // Check for duplicate component types.
            static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
            uint64_t entity = create();
            (assign<T>(entity, get<T>(original)), 
                ..., assign<Ts>(entity, get<Ts>(original)));
            return entity;
        }

        /**
         * @brief Clones n-many entities.
         * 
         * @tparam T The first component type to copy from the original.
         * @tparam Ts The other component types to copy from the original.
         * @param original The entity to copy from.
         * @param n The number of clones to create.
         */
        template <typename T,typename... Ts>
            requires (std::copyable<T> && ... && std::copyable<Ts>)
        void clone(uint64_t original, size_t n) {
            for (size_t i = 0; i < n; ++i)
                (void)clone<T, Ts...>(original);
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
        template <typename T, typename... Ts, typename OutIt>
            requires (std::copyable<T> && ... && std::copyable<Ts>)
                && std::output_iterator<OutIt, uint64_t>
                && std::same_as<std::iter_value_t<OutIt>, uint64_t>
        [[maybe_unused]] OutIt clone(uint64_t original, OutIt out, size_t n) {
            auto generate_clone = 
                [this, original](){ return clone<T, Ts...>(original); };
            return std::generate_n(out, n, generate_clone);
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
        template <typename T, typename... Ts, typename FwdIt>
            requires (std::copyable<T> && ... && std::copyable<Ts>)
                && std::forward_iterator<FwdIt>
                && std::same_as<std::iter_value_t<FwdIt>, uint64_t>
        void clone(uint64_t original, FwdIt first, FwdIt last) {
            auto generate_clone = 
                [this, original](){ return clone<T, Ts...>(original); };
            std::generate(first, last, generate_clone);
        }

        /**
         * @brief Checks if an entity belongs to *this.
         * 
         * @param eid The entity to check.
         * @return true If the entity belongs to *this.
         * @return false If the entity does not belong to *this.
         */
        [[nodiscard]] bool valid(uint64_t eid) const {
            auto idx = dtl::index_from_entity(eid);
            auto ver = dtl::version_from_entity(eid);
            return idx < m_versions.size() && m_versions[idx] == ver;
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
        template <typename InIt>
            requires std::input_iterator<InIt>
                && std::same_as<std::iter_value_t<InIt>, uint64_t>
        [[nodiscard]] bool valid(InIt first, InIt last) const {
            auto is_valid = [this](auto e) { return valid(e); };
            return std::all_of(first, last, is_valid);
        }

        /**
         * @brief Destroys an entity.
         * 
         * @param eid The entity to destroy.
         */
        void destroy(uint64_t eid) {
            // Remove all components and leave all groups
            orphan(eid);
            
            // Split the entity into its index and version
            auto idx = dtl::index_from_entity(eid);
            auto ver = dtl::version_from_entity(eid);
            
            // Ensure the entity can still be reused
            assert(ver < 0xFFFFFFFF);

            // Revise the entity & make it reusable
            m_versions[idx] = ver + 1u;
            m_free_list.push(idx);
        }

        /**
         * @brief Destroys a collection of entities.
         * 
         * @tparam InIt An input iterator type.
         * @param first An iterator to the beginning of a range.
         * @param last An iterator to one-past the end of a range.
         */
        template <typename InIt>
            requires std::input_iterator<InIt>
                && std::same_as<std::iter_value_t<InIt>, uint64_t>
        void destroy(InIt first, InIt last) {
            auto destroy_entity = [this](auto e){ destroy(e); };
            std::for_each(first, last, destroy_entity);
        }
        
        /**
         * @brief Removes all components from an entity.
         * 
         * @param eid The entity to orphan.
         */
        void orphan(uint64_t eid) {
            assert(valid(eid));

            auto idx = dtl::index_from_entity(eid);

            // Remove all components.
            for (auto& metabuffer : m_metabuffers) {
                if (idx < metabuffer.size() && metabuffer.test(idx))
                    metabuffer.reset(idx);
            }

            // Remove the entity from all component groups.
            for (auto& [_, group] : m_groups) 
                group.erase(eid);
        }

        /**
         * @brief Removes all components from the entities in the range 
         * [first, last).
         * 
         * @tparam InIt Input iterator type.
         * @param first Beginning of the range.
         * @param last One-past the end of the range.
         */
        template <typename InIt>
            requires std::input_iterator<InIt>
                && std::same_as<std::iter_value_t<InIt>, uint64_t>
        void orphan(InIt first, InIt last) {
            auto orphan_entity = [this](auto e) { orphan(e); };
            std::for_each(first, last, orphan_entity);
        }

        /**
         * @brief Checks if a given entity has all of the 
         * given component types.
         * 
         * @tparam T The first component type to check.
         * @tparam Ts The other component types to check.
         * @param eid The entity to check for.
         * @return true If the given entity has all of the 
         * given component types.
         * @return false If the given entity does not have
         * all of the given component types.
         */
        template <typename T,typename... Ts>
        [[nodiscard]] bool has(uint64_t eid) const {
            if constexpr (sizeof...(Ts) == 0) {
                // Any invalid entity will never have a component managed by
                // *this. Inversely, any valid entity will never own a component
                // unmanaged by this.
                if (!valid(eid) || !contains<T>())
                    return false;
                auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());
                auto idx = dtl::index_from_entity(eid);
                const auto& metabuffer = m_metabuffers[buffer_index];
                return idx < metabuffer.size() && metabuffer.test(idx);
            }
            else {
                // Check for duplicate component types
                static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
                return (has<T>(eid) && ... && has<Ts>(eid));
            }
        }

        /**
         * @brief Removes components from an entity.
         * 
         * @tparam T The first component type to remove.
         * @tparam Ts The other component types to remove.
         * @param eid The entity to remove from.
         */
        template <typename T, typename... Ts>
        void remove(uint64_t eid) {
            if constexpr (sizeof...(Ts) == 0) {
                assert(valid(eid));
                assert(has<T>(eid));

                auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());

                // Disable the component for the entity.
                auto& metabuffer = m_metabuffers[buffer_index];
                metabuffer.reset(dtl::index_from_entity(eid));

                // Remove the entity from all groups for which it no longer 
                // shares a common set of components. The search for newly
                // nonapplicable groups is limited by the remove component.
                for (auto& [filter, group] : m_groups)
                    if (buffer_index < filter.size() 
                            && filter.test(buffer_index))
                        group.erase(eid);
            }
            else {
                // Check for duplicate component types
                static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
                (remove<T>(eid), ..., remove<Ts>(eid));
            }
        }

        /**
         * @brief Removes components from a collection of entities.
         * 
         * @tparam T The first component type to remove.
         * @tparam Ts The other component types to remove.
         * @tparam InIt Input iterator type.
         * @param first Beginning of the range of entities to remove 
         * components from.
         * @param last One-past the end of the range of entities to 
         * remove components from.
         */
        template <typename T, typename... Ts, typename InIt>
            requires std::input_iterator<InIt>
                && std::same_as<std::iter_value_t<InIt>, uint64_t>
        void remove(InIt first, InIt last) {
            auto remove_components = [this](auto e){ remove<T, Ts...>(e); };
            std::for_each(first, last, remove_components);
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
        template <typename T, typename... Args>
            requires std::movable<T> && std::constructible_from<T, Args...>
        [[maybe_unused]] decltype(auto) assign(uint64_t eid, Args&&... args) {
            assert(valid(eid) && "The entity does not belong to *this.");
            assert(!has<T>(eid) && 
                "The entity already has the component.");

            // Retreive the index to the component's buffers.
            auto buffer_index = accommodate<T>();

            // Retreive the entity's index to the component's buffers.
            auto idx = dtl::index_from_entity(eid);

            // Retrieve the component metabuffer for the given type.
            auto& metabuffer = m_metabuffers[buffer_index];

            // Ensure there exists component metadata for the entity.
            if (idx >= metabuffer.size())
                metabuffer.resize(idx + 1);

            // Logically add the component to the entity.
            metabuffer.set(idx);

            // Add the entity to any group which it now shares a common set of 
            // components with. The search for applicable groups is limited by
            // by the recently added component.
            for (auto& [filter, group] : m_groups) {
                // Skip groups which already contain this entity.
                if (group.contains(eid))
                    continue;
                // Skip groups which do not include the new component.
                if (buffer_index >= filter.size()
                        || !filter.test(buffer_index))
                    continue;

                // In order to check if an entity belongs to a group
                // We must ensure that for all active bits in the group's 
                // bitset, there exists an active component associated with
                // the entity we're trying to add.
                bool has_all = true;
                size_t i = filter.find_first();
                for (; i < filter.size(); i = filter.find_next(i)) {
                    // Check if there is a set bit in the metabuffer at
                    // the entity's index.
                    if (idx >= m_metabuffers[i].size()
                            || !m_metabuffers[i].test(idx))
                    {
                        has_all = false;
                        break;
                    }
                }
                if (has_all)
                    group.insert(eid);
            }

            // Retrieve the component buffer for the given type.
            auto& buffer = 
                std::any_cast<std::vector<T>&>(m_buffers[buffer_index]);

            // Ensure there physically exists memory for the new component
            if (idx >= buffer.size())
                buffer.resize(idx + 1);

            // Construct and return the component.
            if constexpr (std::is_aggregate_v<T>)
                buffer[idx] = T{std::forward<Args>(args)...};
            else
                buffer[idx] = T(std::forward<Args>(args)...);
            return buffer[idx];
        }

        /**
         * @brief Assigns components to a collection of entities.
         * 
         * @tparam T The first component type to assign.
         * @tparam Ts The other component types to assign.
         * @tparam InIt Input iterator type. 
         * @param first Beginning of the range of entities to assign 
         * components to.
         * @param last One-past the end of the range of entities to 
         * assign components to.
         */
        template <typename T, typename... Ts, typename InIt>
            requires (std::movable<T> && ... && std::movable<Ts>)
                && (std::default_initializable<T> 
                        && ... && std::default_initializable<Ts>)
                && std::input_iterator<InIt>
                && std::same_as<std::iter_value_t<InIt>, uint64_t>
        void assign(InIt first, InIt last) {
            // Check for duplicate component types
            static_assert(dtl::is_unique(dtl::type_list_v<Ts...>));

            auto assign_components = 
                [this](auto e){ (assign<T>(e), ..., assign<Ts>(e)); };
            std::for_each(first, last, assign_components);
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
        template <typename T, typename... Args>
            requires std::movable<T> && std::constructible_from<T, Args...>
        [[maybe_unused]] 
        decltype(auto) assign_or_replace(uint64_t eid, Args&&... args) {
            assert(valid(eid) && "The entity does not belong to *this.");
            if (!has<T>(eid)) {
                return assign<T>(eid, std::forward<Args>(args)...);
            }
            else {
                auto& current = get<T>(eid);
                if constexpr (std::is_aggregate_v<T>) {
                    T replacement{std::forward<Args>(args)...};
                    std::swap(current, replacement);
                }
                else {
                    T replacement(std::forward<Args>(args)...);
                    std::swap(current, replacement);
                }
                return current;
            }
        }

        /**
         * @brief Returns an entity's components.
         * 
         * @tparam T The first component to retrieve.
         * @tparam Ts The other component types to retrieve.
         * @param eid The entity to fetch for.
         * @return Reference to a component or a tuple of references.
         */
        template <typename T, typename... Ts>
        [[nodiscard]] decltype(auto) get(uint64_t eid) {
            if constexpr (sizeof...(Ts) == 0) {
                if constexpr (std::is_const_v<T>) {
                    return std::as_const(*this).template get<T>(eid);
                }
                else {
                    assert(has<T>(eid));
                    auto buffer_index = 
                        m_buffer_indices.at(dtl::type_index<T>());
                    using buffer_type = std::vector<std::decay_t<T>>&;
                    auto& buffer = 
                        std::any_cast<buffer_type>(m_buffers[buffer_index]);
                    return buffer[dtl::index_from_entity(eid)];
                }
            }
            else {
                static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
                return std::forward_as_tuple(get<T>(eid), get<Ts>(eid)...);
            }
        }

        /*! @copydoc get */
        template <typename T,typename... Ts>
        [[nodiscard]] decltype(auto) get(uint64_t eid) const {
            if constexpr (sizeof...(Ts) == 0) {
                static_assert(std::is_const_v<T>);
                assert(has<T>(eid));
                auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());
                using buffer_type = const std::vector<std::decay_t<T>>&;
                const auto& buffer = 
                    std::any_cast<buffer_type>(m_buffers[buffer_index]);
                return buffer[dtl::index_from_entity(eid)];
            }
            else {
                static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
                return std::forward_as_tuple(get<T>(eid), get<Ts>(eid)...);
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
        [[nodiscard]] size_t count() const {
            auto entities = m_versions | boost::adaptors::indexed();
            auto has_components = [this](const auto& el)  { 
                auto entity = dtl::make_entity(el.value(), el.index());
                return has<T, Ts...>(entity); 
            };
            auto ret = std::ranges::count_if(entities, has_components);
            return static_cast<size_t>(ret);
        }

        /**
         * @brief Returns the maximum number of elements of the given type that
         * *this is able to hold due to system or library implementation 
         * limitations.
         * 
         * @tparam T Component type to check.
         * @return The maximum number of elements.
         */
        template <typename T>
        [[nodiscard]] size_t max_size() const {
            assert(contains<T>());
            auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());
            using buffer_type = const std::vector<T>&;
            const auto& buffer = 
                std::any_cast<buffer_type>(m_buffers[buffer_index]);
            return buffer.max_size();
        }

        /**
         * @brief Returns the number of in-use elements of the given type.
         * 
         * @tparam T Component type of the component vector.
         * @return The number of elements in the compnent vector.
         */
        template <typename T>
        [[nodiscard]] size_t size() const {
            assert(contains<T>());
            auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());
            return m_metabuffers[buffer_index].count();
        }

        /**
         * @brief Checks if there are any in-use elements of the given type.
         * 
         * @tparam T Component type of the component vector.
         * @return true if the component vector is empty.
         * @return false if the component vector is not empty.
         */
        template <typename T>
        [[nodiscard]] bool empty() const {
            assert(contains<T>());
            auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());
            return m_metabuffers[buffer_index].none();
        }

        /**
         * @brief Returns the number of elements that can be held in currently
         * allocated storage for the given type.
         * 
         * @tparam T The given component type.
         * @return The number of elements that can be held.
         */
        template <typename T>
        [[nodiscard]] size_t capacity() const {
            assert(contains<T>());
            auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());
            using buffer_type = const std::vector<T>&;
            const auto& buffer = 
                std::any_cast<buffer_type>(m_buffers[buffer_index]);
            return buffer.capacity();
        }

        /**
         * @brief Requests the removal of unused capacity for each of the given
         * types.
         * 
         * @tparam T The first component type to remove unused capacity for.
         * @tparam Ts The other component types to remove unused capacity for.
         */
        template <typename T, typename... Ts>
        void shrink_to_fit() {
            if constexpr (sizeof...(Ts) == 0) {
                assert(contains<T>());
                auto buffer_index = m_buffer_indices.at(dtl::type_index<T>());
                // Request removal of unused capacity from the component
                // buffer.
                using buffer_type = std::vector<T>&;
                auto& buffer = 
                    std::any_cast<buffer_type>(m_buffers[buffer_index]);
                buffer.shrink_to_fit();
                // Request removal of unused capacity from the component 
                // meta buffer.
                auto& metabuffer = m_metabuffers[buffer_index];
                metabuffer.shrink_to_fit();
            }
            else {
                static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
                (shrink_to_fit<T>(), ..., shrink_to_fit<Ts>());
            }
        }

        /**
         * @brief Reserves storage space for the given component types.
         * 
         * @tparam T The first component type to reserve space for.
         * @tparam Ts The other component types to reserve space for.
         * @param n The number of components to allocate space for.
         */
        template <typename T, typename... Ts>
        void reserve(size_t n) {
            if constexpr (sizeof...(Ts) == 0) {
                auto buffer_index = accommodate<T>();
                // Reserve memory in the compnent buffer.
                using buffer_type = std::vector<T>&;
                auto& buffer = 
                    std::any_cast<buffer_type>(m_buffers[buffer_index]);
                buffer.reserve(n);
                // Reserve memory in the component metabuffer.
                auto& metabuffer = m_metabuffers[buffer_index];
                metabuffer.reserve(n);
            }
            else {
                static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
                (reserve<T>(n), ..., reserve<Ts>(n));
            }
        }

        /**
         * @brief Checks if the given types are managed by *this.
         * 
         * @tparam T The first component type to check.
         * @tparam Ts The other component types to check.
         * @return true If *this manages all of the given types.
         * @return false If *this does not manage all of the given types.
         */
        template <typename T, typename... Ts>
        [[nodiscard]] bool contains() const {
            if constexpr (sizeof...(Ts) == 0) {
                auto type_index = dtl::type_index<T>();
                auto iterator = m_buffer_indices.find(type_index);
                return iterator != m_buffer_indices.end();
            }
            else {
                static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));
                return (contains<T>() && ... && contains<Ts>());
            }
        }

        /**
         * @brief Returns the number of types that *this manages.
         * 
         * @return The number of managed types.
         */
        [[nodiscard]] size_t num_contained_types() const noexcept {
            return static_cast<size_t>(m_buffer_indices.size());
        }

        /**
         * @brief Returns a view of a given set of components.
         * The entities matching the given set of components are cached for
         * fast subsequent retrieval.
         * 
         * @tparam T The first compnent types to view.
         * @tparam Ts The other component types to view.
         * @return An instance of ecfw::view.
         */
        template <typename T, typename... Ts>
        [[nodiscard]] ecfw::view<T, Ts...> view() {
            // Check for duplicate component types
            static_assert(dtl::is_unique(dtl::type_list_v<T, Ts...>));

            auto buffer_indices = 
                std::make_tuple(accommodate<T>(), accommodate<Ts>()...);

            auto make_view = [this](auto t, auto... ts) {
                return ecfw::view<T, Ts...>
                {
                    group_by({ t, ts... }),
                    std::any_cast<dtl::buffer_type<T>&>(m_buffers[t]),
                    std::any_cast<dtl::buffer_type<Ts>&>(m_buffers[ts])...
                };
            };
            return std::apply(make_view, buffer_indices);
        }

    private:

        // Ensures that *this can manage the given component types.
        template <typename T>
        [[nodiscard]] size_t accommodate() {
            auto type_index = dtl::type_index<T>();
            auto iterator = m_buffer_indices.find(type_index);
            if (iterator == m_buffer_indices.end()) [[unlikely]] {
                // Map the new type to an index into both m_buffers and 
                // m_metabuffers. This index also serves as a bit position
                // in group identifying bitsets
                size_t buffer_index = 
                    static_cast<size_t>(m_buffer_indices.size());
                m_buffer_indices.emplace(type_index, buffer_index);
                m_buffers.emplace_back(
                    std::make_any<std::vector<std::decay_t<T>>>());
                m_metabuffers.emplace_back();
                assert(m_buffer_indices.size() == m_buffers.size());
                assert(m_buffer_indices.size() == m_metabuffers.size());
                return buffer_index;
            }
            return iterator->second;
        }
        
        [[nodiscard]] const dtl::sparse_set& 
        group_by(const std::initializer_list<size_t>& buffer_indices) {
            // Find the largest type position. Size of the group id is +1.
            size_t largest_buffer_index = std::max(buffer_indices);

            // Ensure we're not working with any unknown components.
            // Type indices are created in sequential order upon 
            // discovery by any world either by assignment, view
            // creation, or storage reservation.
            assert(largest_buffer_index < m_buffer_indices.size());
            assert(largest_buffer_index < m_metabuffers.size());
            assert(largest_buffer_index < m_buffers.size());

            // Build the filter in order to find an existing 
            // group or to create one.
            boost::dynamic_bitset<> filter(largest_buffer_index + 1);
            for (auto buffer_index : buffer_indices)
                filter.set(buffer_index);

            // Check if there exists a group identified by our filter.
            // If so, return it to the caller.
            auto it = m_groups.find(filter);
            if (it != m_groups.end()) [[likely]]
                return it->second;

            // Build the initial group of entities.
            dtl::sparse_set group{};
            uint32_t index = 0;
            for (auto version : m_versions) {
                bool has_all = true;
                for (auto buffer_index : buffer_indices) {
                    const auto& metabuffer = m_metabuffers[buffer_index];
                    if (index >= metabuffer.size() 
                            || !metabuffer.test(index))
                    {
                        has_all = false;
                        break;
                    }
                }
                auto entity = dtl::make_entity(version, index++);
                if (has_all)
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

        // A map from type to valid indices within m_buffers and m_metabuffers.
        std::unordered_map<std::type_index, size_t> m_buffer_indices{};

        // Component metabuffer; one bitset for each component type. 
        // A bitset is only allocated for a component when it's first 
        // assigned to an entity, when a view is created, or when space 
        // is reserved for a component1. W.r.t assignment, only enough 
        // space within the bitset is allocated to accommodate at most
        // the entity involved in the assignment. 
        std::vector<dtl::metabuffer_type> m_metabuffers{};

        // Component data. Space is allocated similarly to the 
        // metabuffers collection. We use a std::any to hide
        // the buffer type until we're in a context where we can
        // deduce the type of the buffer.
        std::vector<std::any> m_buffers{};

        // Filtered groups of entities. Each filter represents a common
        // set of components each entity of the group possesses.
        std::unordered_map<boost::dynamic_bitset<>, dtl::sparse_set> m_groups{};

    };

} // namespace ecfw