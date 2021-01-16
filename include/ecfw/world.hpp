#pragma once 

#include <stack>
#include <cstddef>
#include <vector>
#include <concepts>
#include <algorithm>
#include <unordered_map>
#include <type_traits>
#include <boost/core/noncopyable.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <ecfw/entity.hpp>
#include <ecfw/view.hpp>
#include <ecfw/detail/buffer.hpp>
#include <ecfw/detail/type_index.hpp>
#include <ecfw/detail/type_list.hpp>

namespace ecfw 
{

    /**
     * @brief The class template basic_world stores and manipulates entities 
     * and their components. The class is not dependent on the entity type \p E.
     * The definitions of the operations used on an entity type are supplied 
     * via the \p Traits template parameter - a specialization of 
     * ecfw::entity_traits, or a compatible traits class.
     * 
     * @tparam E One of \p uint64_t, \p uint32_t, \p uint16_t.
     * @tparam Traits traits class specifying the operations on the entity type.
     */
    template <Entity E, typename Traits = entity_traits<E>>
        requires std::same_as<E, typename Traits::entity_type>
    class basic_world : private boost::noncopyable {

        // Used in basic_world::internal_get to select the right buffer type 
        // based on whether or not the requested component type is const.
        // When the type is known to be const or non const, explicit usage
        // of detail::buffer<T> is required.
        template <typename T>
        using buffer_type = std::conditional_t<
            std::is_const_v<T>, 
            const detail::buffer<T>, 
            detail::buffer<T>
        >;

    public:

        using traits_type = Traits;
        using entity_type = typename traits_type::entity_type;
        
        template <typename T, typename... Ts>
        using view_type = basic_view<entity_type, traits_type, T, Ts...>;

        /**
         * @brief Default constructor.
         * 
         */
        basic_world() = default;

        /**
         * @brief Default move constructor.
         * 
         */
        basic_world(basic_world&&) = default;

        /**
         * @brief Default move assignment.
         * 
         * @return *this.
         */
        basic_world& operator=(basic_world&&) = default;

        /**
         * @brief Constructs a new entity.
         * 
         * @note Any given components must be default constructible.
         * 
         * @tparam Ts Initializing component types.
         * @return A newly constructed entity.
         */
        template <std::default_initializable... Ts>
        [[nodiscard]] entity_type create() {
            auto bids = std::make_tuple(get_or_create_buffer_index<Ts>()...);
            return internal_create<Ts...>(bids);
        }

        /**
         * @brief Constructs \p n entities.
         * 
         * @note Any given components must be default constructible.
         * 
         * @tparam Ts Initializing component types.
         */
        template <std::default_initializable... Ts>
        void create(std::size_t n) {
            auto bids = std::make_tuple(get_or_create_buffer_index<Ts>()...);
            for (std::size_t i = 0; i != n; ++i)
                (void)internal_create<Ts...>(bids);
        }

        /**
         * @brief Assigns each element in the range [\p first, \p first + \p n)
         * an entity constructed by \p *this.
         * 
         * @note Any given components must be default constructible.
         * 
         * @tparam OutIt Output iterator type
         * @tparam Ts Initializing component types.
         * @param first An output iterator.
         * @param n The number of elements to construct.
         * @return The given output iterator.
         */
        template <
            std::default_initializable... Ts,
            std::output_iterator<entity_type> OutIt
        >
        [[maybe_unused]] OutIt create(OutIt first, std::size_t n) {
            auto bids = std::make_tuple(get_or_create_buffer_index<Ts>()...);
            return std::generate_n(first, n, [this, &bids]() {
                return internal_create<Ts...>(bids);
            });
        }

        /**
         * @brief Assigns each element in ther range [\p first, \p last) an 
         * entity constructed by \p *this.
         * 
         * @note Any given components must be default constructible.
         * 
         * @tparam FwdIt A forward iterator type.
         * @tparam Ts Initializing component types.
         * @param first The beginning of a range.
         * @param last One-past the end of a range.
         */
        template <std::default_initializable... Ts, std::forward_iterator FwdIt>
            requires std::same_as<std::iter_value_t<FwdIt>, entity_type>
        void create(FwdIt first, FwdIt last) {
            auto bids = std::make_tuple(get_or_create_buffer_index<Ts>()...);
            std::generate(first, last, [this, &bids]() {
                return internal_create<Ts...>(bids);
            });
        }

        /**
         * @brief Constructs a new entity as a clone of another.
         * 
         * @note Any components given for cloning must be copy constructible.
         * 
         * @tparam T The first component to copy.
         * @tparam Ts Other components to copy.
         * @param base The entity to copy from.
         * @return A newly constructed entity.
         */
        template <std::copyable T, std::copyable... Ts>
        [[nodiscard]] entity_type clone(entity_type base) {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            return internal_clone<T, Ts...>(bids, base);
        }

        /**
         * @brief Constructs \n clones on another entity.
         * 
         * @note Any components given for cloning must be copy constructible.
         * 
         * @tparam T The first component to copy.
         * @tparam Ts Other components to copy.
         * @param base The entity to copy from.
         * @param n The number of clones to create.
         */
        template <std::copyable T, std::copyable... Ts>
        void clone(entity_type base, std::size_t n) {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            for (std::size_t i = 0; i != n; ++i)
                (void)internal_clone<T, Ts...>(bids, base);
        }

        /**
         * @brief Assigns each element in the range [\p first, \p first + \p n)
         * a clone constructed by \p *this.
         * 
         * @tparam OutIt An output iterator type.
         * @tparam T The first component to copy.
         * @tparam Ts Other components to copy.
         * @param base The entity to copy from.
         * @param first The beginning of a range.
         * @param n The number of clones to create.
         * @return The given output iterator.
         */
        template <
            std::copyable T, 
            std::copyable... Ts,
            std::output_iterator<entity_type> OutIt
        >
        [[maybe_unused]] OutIt clone(entity_type base, OutIt first, std::size_t n) {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            return std::generate_n(first, n, [this, &bids, base]() {
                return internal_clone<T, Ts...>(bids, base);
            });
        }

        /**
         * @brief Assigns each element in the range [\p first, \p last) a clone
         * constructed by \p *this.
         * 
         * @tparam FwdIt Forward iterator type.
         * @tparam T The first component to copy.
         * @tparam Ts The other components to copy.
         * @param base The entity to copy from.
         * @param first The beginning of a range.
         * @param last One-past the end of a range.
         */
        template <
            std::copyable T, 
            std::copyable... Ts,
            std::forward_iterator FwdIt
        > 
            requires std::same_as<std::iter_value_t<FwdIt>, entity_type>
        void clone(entity_type base, FwdIt first, FwdIt last) {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            std::generate(first, last, [this, &bids, base]() {
                return internal_clone<T, Ts...>(bids, base);
            });
        }

        /**
         * @brief Removes all components from an entity.
         * 
         * @param e The entity to orphan.
         */
        void orphan(entity_type e) {
            assert(is_valid(e));
            auto eid = traits_type::index(e);
            // Deactivate all of the entity's components.
            for (auto& metabuffer : m_metabuffers) {
                if (eid < metabuffer.size() && metabuffer.test(eid))
                    metabuffer.reset(eid);
            }
            // Remove the entity from all groups.
            for (auto& [_, group] : m_groups) {
                if (group.contains(e))
                    group.erase(eid);
            }
        }

        /**
         * @brief Removes all components from entities in the range
         * [\p first, \p last).
         * 
         * @tparam InIt An input iterator type.
         * @param first The beginning of a range.
         * @param last One-past the end of a range.
         */
        template <std::input_iterator InIt>
            requires std::same_as<std::iter_value_t<InIt>, entity_type>
        void orphan(InIt first, InIt last) {
            std::for_each(first, last, [this](auto e) { orphan(e); });
        }

        /**
         * @brief Checks if an entity does not have any components.
         * 
         * @param e The entity check for.
         * @return true If the entity does not have any components.
         * @return false If the entiy has at least one comopnent.
         */
        [[nodiscard]] bool is_orphan(entity_type e) const {
            return std::ranges::none_of(m_metabuffers, 
            [this, eid = traits_type::index(e)](const auto& metabuffer) {
                return eid < metabuffer.size() && metabuffer.test(eid);
            });
        }

        /**
         * @brief Applies a functor on each orphaned entity.
         * 
         * @tparam UnaryFunction Functor type.
         * @param f The functor to apply.
         */
        template <std::invocable<entity_type> UnaryFunction>
        void orphans(UnaryFunction f) {
            std::ranges::for_each(m_entities, [this, &f](auto e) {
                if (is_orphan(e))
                    f(e);
            });
        }

        /**
         * @brief Removes all components and recycles an entity.
         * 
         * @param e The entiy to destroy.
         */
        void destroy(entity_type e) {
            orphan(e);
            recycle(e);
        }

        /**
         * @brief Destroys the entities in the range [\p first, \p last).
         * 
         * @tparam InIt An input iterator type.
         * @param first The beginning of a range.
         * @param last One-past the end of a range.
         */
        template <std::input_iterator InIt>
            requires std::same_as<std::iter_value_t<InIt>, entity_type>
        void destroy(InIt first, InIt last) {
            std::for_each(first, last, [this](auto e) { destroy(e); });
        }

        /**
         * @brief Checks if an entity belongs to \p *this.
         * 
         * @param e The entity to check.
         */
        [[nodiscard]] bool is_valid(entity_type e) const {
            auto eid = traits_type::index(e);
            assert(eid < num_entities());
            return m_entities[eid] == e;
        }

        /**
         * @brief Checks if each entity in the range [\p first, \p last)
         * belongs to \p *this.
         * 
         * @tparam InIt An input iterator type.
         * @param first The beginning of a range.
         * @param last One-past the end of a range.
         * @return true If all entities in the range are valid.
         * @return false If there exists an entity in the range that is invalid.
         */
        template <std::input_iterator InIt>
            requires std::same_as<std::iter_value_t<InIt>, entity_type>
        [[nodiscard]] bool is_valid(InIt first, InIt last) const {
            auto predicate = [this](auto e) { return is_valid(e); };
            return std::all_of(first, last, predicate);
        }

        /**
         * @brief Checks if an entity has any component(s).
         * 
         * @tparam T The first component to check.
         * @tparam Ts The other components to check.
         * @param e The entity to check.
         * @return true If the entity has all of given components.
         * @return false If the entity is missing any of the given components.
         */
        template <typename T, typename... Ts>
        [[nodiscard]] bool has(entity_type e) const {
            return (internal_has(get_buffer_index<T>(), e)
                && ... && internal_has(get_buffer_index<Ts>(), e));
        }
        
        /**
         * @brief Deactivates an entity's component(s).
         * 
         * @tparam T The first component to deactivate.
         * @tparam Ts The other components to deactivate.
         * @param e The entity to remove components for.
         */
        template <typename T, typename... Ts>
        void remove(entity_type e) {
            (internal_remove(get_buffer_index<T>(), e), 
                ..., internal_remove(get_buffer_index<Ts>(), e));
        }

        /**
         * @brief Deactivates components from a range of entities.
         * 
         * @tparam InIt An input iterator type.
         * @tparam T The first component type to deactivate.
         * @tparam Ts The other component types to deactivate.
         * @param first The beginning of a range.
         * @param last One-past the end of a range.
         */
        template <typename T, typename... Ts, std::input_iterator InIt>
            requires std::same_as<std::iter_value_t<InIt>, entity_type>
        void remove(InIt first, InIt last) {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            std::for_each(first, last, [this, &bids](auto e) {
                std::apply([this, e](auto b, auto... bs) {
                    (internal_remove(b, e), ..., internal_remove(bs, e));
                }, bids);
            });
        }

        /**
         * @brief Assigns an entity a component.
         * 
         * @tparam T The component type to assign.
         * @tparam Args Constructor parameter types.
         * @param e The entity to assign to.
         * @param args Constructor parameters.
         * @return A reference to the entity's component.
         */
        template <std::movable T, typename... Args>
            requires std::constructible_from<T, Args...>
        [[maybe_unused]] decltype(auto) assign(entity_type e, Args&&... args) {
            auto bid = get_or_create_buffer_index<T>();
            return internal_assign<T>(bid, e, std::forward<Args>(args)...);
        }

        /**
         * @brief Assigns components to a range of entities.
         * 
         * @tparam InIt An input iterator type.
         * @tparam T The first component type to assign.
         * @tparam Ts The other component types to assign.
         * @param first The beginning of a range.
         * @param last One-past the end of a range.
         */
        template <
            std::default_initializable T, 
            std::default_initializable... Ts,
            std::input_iterator InIt
        >
        void assign(InIt first, InIt last) {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            std::for_each(first, last, [this, &bids](auto e) {
                std::apply([this, e](auto b, auto... bs) {
                    (internal_assign<T>(b, e), ..., internal_assign<Ts>(bs, e));
                }, bids);
            });
        }

        /**
         * @brief Assigns a component to an entity.
         * 
         * @tparam T The component type to assign.
         * @tparam Args Contructor parameter types.
         * @param e The entity to assign to.
         * @param args Component constructor parameters.
         * @return A reference to the entity's component.
         */
        template <std::movable T, typename... Args>
            requires std::constructible_from<T, Args...>
        [[maybe_unused]] decltype(auto) 
        assign_or_replace(entity_type e, Args&&... args) {
            auto bid = get_or_create_buffer_index<T>();
            if (!internal_has(bid, e)) {
                return internal_assign<T>(bid, e, std::forward<Args>(args)...);
            }
            else {
                auto& current = internal_get<T>(bid, e);
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
         * @brief Gets an entity's component(s).
         * 
         * @tparam T The first component to obtain.
         * @tparam Ts The other component(s) to obtain.
         * @param e The owning entity.
         * @return A reference or tuple of references.
         */
        template <typename T, typename... Ts>
        [[nodiscard]] decltype(auto) get(entity_type e) {
            if constexpr (sizeof...(Ts) == 0)
                return internal_get<T>(get_buffer_index<T>(), e);
            else 
                return std::forward_as_tuple(get<T>(e), get<Ts>(e)...);
        }

        /*! @copydoc get */
        template <typename T, typename... Ts>
            requires std::conjunction_v<std::is_const<T>, std::is_const<Ts>...>
        [[nodiscard]] decltype(auto) get(entity_type e) const {
            if constexpr (sizeof...(Ts) == 0)
                return internal_get<T>(get_buffer_index<T>(), e);
            else 
                return std::forward_as_tuple(get<T>(e), get<Ts>(e)...);
        }

        /**
         * @brief Returns the size of a component buffer.
         * 
         * @tparam T The component type.
         * @return The number of constructed components.
         */
        template <typename T>
        [[nodiscard]] std::size_t size() const {
            auto bid = get_buffer_index<T>();
            auto buffer = 
                static_cast<const detail::buffer<T>*>(m_buffers[bid].get());
            return buffer->size();
        }

        /**
         * @brief Checks if a component buffer is empty.
         * 
         * @tparam T The component type to check.
         * @return true If there are no components constructed.
         * @return false If there are elements constructed.
         */
        template <typename T>
        [[nodiscard]] bool empty() const {
            auto bid = get_buffer_index<T>();
            auto buffer = 
                static_cast<const detail::buffer<T>*>(m_buffers[bid].get());
            return buffer->empty();
        }

        /**
         * @brief Returns the capacity of a component buffer.
         * 
         * @tparam T The given component type.
         * @return The number of components that can be held in current storage.
         */
        template <typename T>
        [[nodiscard]] std::size_t capacity() const {
            auto bid = get_buffer_index<T>();
            auto buffer = 
                static_cast<const detail::buffer<T>*>(m_buffers[bid].get());
            return buffer->capacity();
        }

        /**
         * @brief Reserves storage space for the given component types.
         * 
         * @tparam T The first component type to reserve space for.
         * @tparam Ts The other component types to reserve space for.
         * @param n The number of components to allocate space for.
         */
        template <typename T, typename... Ts>
        void reserve(std::size_t n) {
            if constexpr (sizeof... (Ts) == 0) {
                auto bid = get_or_create_buffer_index<T>();
                auto buffer = 
                    static_cast<detail::buffer<T>*>(m_buffers[bid].get());
                buffer->reserve(n);
                m_metabuffers[bid].reserve(n);
            }
            else {
                (reserve<T>(n), ..., reserve<Ts>(n));
            }
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
            if constexpr (sizeof... (Ts) == 0) {
                auto bid = get_buffer_index<T>();
                auto buffer = 
                    static_cast<detail::buffer<T>*>(m_buffers[bid].get());
                buffer->shrink_to_fit();
                m_metabuffers[bid].shrink_to_fit();
            }
            else {
                (shrink_to_fit<T>(), ..., shrink_to_fit<Ts>());
            }
        }

        template <typename T, typename... Ts>
        [[nodiscard]] std::size_t count() const {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            auto predicate = [this, &bids](auto e) {
                return std::apply([this, e](auto t, auto... ts){
                    return (internal_has(t, e) && ... && internal_has(ts, e));
                }, bids);
            };
            return std::ranges::count_if(m_entities, predicate);
        }

        /**
         * @brief Returns a view of a given set of components.
         * 
         * @tparam T The first component type to view.
         * @tparam Ts The other component types to view.
         * @return An entity view.
         */
        template <typename T, typename... Ts>
        [[nodiscard]] ecfw::basic_view<E, Traits, T, Ts...> view() {
            auto bids = std::make_tuple(
                get_or_create_buffer_index<T>(), 
                get_or_create_buffer_index<Ts>()...
            );
            auto make_view = [this](auto b, auto... bs) {
                return ecfw::basic_view<E, Traits, T, Ts...>{
                    group_by({b, bs...}), 
                    *static_cast<buffer_type<T>*>(m_buffers[b].get()),
                    *static_cast<buffer_type<Ts>*>(m_buffers[bs].get())...
                };
            };
            return std::apply(make_view, bids);
        }

        /*! @copydoc view */
        template <typename T, typename... Ts>
            requires std::conjunction_v<std::is_const<T>, std::is_const<Ts>...>
        [[nodiscard]] ecfw::basic_view<E, Traits, T, Ts...> view() const {
            auto bids = std::make_tuple(
                get_buffer_index<T>(), get_buffer_index<Ts>()...);
            auto make_view = [this](auto b, auto... bs) {
                return ecfw::basic_view<E, Traits, T, Ts...>{
                    group_by({b, bs...}), 
                    *static_cast<const detail::buffer<T>*>(m_buffers[b].get()),
                    *static_cast<const detail::buffer<Ts>*>(m_buffers[bs].get())...
                };
            };
            return std::apply(make_view, bids);
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
            return (m_buffer_indices.contains(detail::type_index::type_id<T>()) 
                    && ... && 
                m_buffer_indices.contains(detail::type_index::type_id<Ts>()));
        }

        /**
         * @brief Returns the number of types that *this manages.
         * 
         * @return The number of managed types.
         */
        [[nodiscard]] std::size_t num_contained_types() const {
            return m_buffer_indices.size();
        }

        /**
         * @brief Returns the number of created entities.
         * 
         * @return The number of created entities.
         */
        [[nodiscard]] std::size_t num_entities() const {
            return m_entities.size();
        }

        /**
         * @brief Returns the number of entities that are inactive.
         * 
         * @return The number entities that can be reused.
         */
        [[nodiscard]] std::size_t num_inactive() const {
            return m_inactive.size();
        }

        /**
         * @brief Returns the number of entities that have not been destroyed.
         * 
         * @return The number of entities that have not been destroyed.
         */
        [[nodiscard]] std::size_t num_alive() const {
            return num_entities() - num_inactive();
        }

    private:

        [[nodiscard]] entity_type generate_entity() {
            entity_type e{};
            if (m_inactive.empty()) {
                using index_type = typename traits_type::index_type;
                using version_type = typename traits_type::version_type;
                auto eid = static_cast<index_type>(m_entities.size());
                e = traits_type::combine(eid, version_type{0});
                m_entities.emplace_back(e);
            }
            else {
                e = m_entities[m_inactive.top()];
                m_inactive.pop();
            }
            assert(num_inactive() <= num_entities());
            return e;
        }

        void recycle(entity_type e) {
            assert(is_valid(e));
            assert(traits_type::is_recyclable(e));
            auto eid = traits_type::index(e);
            m_entities[eid] = 
                traits_type::combine(eid, traits_type::version(e) + 1);
            m_inactive.push(std::size_t{eid});
            assert(num_inactive() <= num_entities());
            assert(!is_valid(e));
        }

        // The following internal_* functions exist to reduce the number of
        // queries made to `m_buffer_indices` in situations which operate on 
        // ranges.

        template <typename... Ts, typename BufferIndexTuple>
        [[nodiscard]] entity_type internal_create(BufferIndexTuple&& bids) {
            entity_type e = generate_entity();
            std::apply([this, e](auto... bs) {
                (internal_assign<Ts>(bs, e), ...);
            }, bids);
            return e;
        }

        template <typename T, typename... Ts, typename BufferIndexTuple>
        [[nodiscard]] entity_type 
        internal_clone(BufferIndexTuple&& bids, entity_type base) {
            entity_type e = generate_entity();
            std::apply([this, base, e](auto b, auto... bs) {
                (internal_assign<T>(b, e, internal_get<T>(b, base)), 
                    ..., 
                    internal_assign<Ts>(bs, e, internal_get<Ts>(bs, base)));
            }, bids);
            return e;
        }

        [[nodiscard]] bool internal_has(std::size_t bid, entity_type e) const {
            assert(is_valid(e));
            auto eid = traits_type::index(e);
            const auto& metabuffer = m_metabuffers[bid];
            return eid < metabuffer.size() && metabuffer.test(eid);
        }

        void internal_remove(std::size_t bid, entity_type e) {
            assert(internal_has(bid, e));
            auto eid = traits_type::index(e);
            auto& metabuffer = m_metabuffers[bid];

            // Deactive the entity's component.
            metabuffer.reset(eid);

            // Remove the entity from all groups for which it no longer 
            // shares a common set of components. The search for newly
            // nonapplicable groups is limited by the remove component.
            for (auto& [gid, group] : m_groups) {
                if (bid < gid.size() && gid.test(bid) && group.contains(e))
                    group.erase(e);
            }
        }

        template <typename T, typename... Args>
        [[maybe_unused]] decltype(auto) 
        internal_assign(std::size_t bid, entity_type e, Args&&... args) {
            assert(!internal_has(bid, e));

            auto eid = traits_type::index(e);
            auto& metabuffer = m_metabuffers[bid];

            // Accommodate the entity as necessary.
            if (eid >= metabuffer.size())
                metabuffer.resize(eid + 1);

            // Activate the component.
            metabuffer.set(eid);

            // Add the entity to any group which it now shares a common set of 
            // components with. The search for applicable groups is limited by
            // by the recently added component.
            for (auto& [gid, group] : m_groups) {
                // Skip groups which already include this entity.
                if (group.contains(e))
                    continue;
                // Skip groups which do not include the target component.
                if (bid >= gid.size() || !gid.test(bid))
                    continue;

                bool satisfied = true;
                // For each component type, check that the entity has a 
                // component of that type...
                std::size_t bid = gid.find_first();
                for (; bid < gid.size(); bid = gid.find_next(bid)) {
                    const auto& metabuffer = m_metabuffers[bid];
                    // The entity does not have a required component.
                    if (eid >= metabuffer.size() || !metabuffer.test(eid)) {
                        satisfied = false;
                        break;
                    }
                }
                if (satisfied)
                    group.insert(e);
            }

            auto buffer = 
                static_cast<detail::buffer<T>*>(m_buffers[bid].get());
            
            // Accommodate the entity as necessary.
            if (eid >= buffer->size())
                buffer->resize(eid + 1);

            // Construct the component.
            if constexpr (std::is_aggregate_v<T>)
                (*buffer)[eid] = T{std::forward<Args>(args)...};
            else
                (*buffer)[eid] = T(std::forward<Args>(args)...);
            return (*buffer)[eid];
        }

        template <typename T>
        [[nodiscard]] decltype(auto) 
        internal_get(std::size_t bid, entity_type e) {
            assert(internal_has(bid, e));
            auto buffer = 
                static_cast<buffer_type<T>*>(m_buffers[bid].get());
            return (*buffer)[traits_type::index(e)];
        }

         template <typename T>
        [[nodiscard]] decltype(auto)
        internal_get(std::size_t bid, entity_type e) const {
            assert(internal_has(bid, e));
            auto buffer = 
                static_cast<const detail::buffer<T>*>(m_buffers[bid].get());
            return (*buffer)[traits_type::index(e)];
        }

        // Used in situations where a buffer index may need to be constructed.
        // i.e., when we need to construct a component, reserve a certain 
        // number of components, or when we construct new views.
        template <typename T>
        [[nodiscard]] std::size_t get_or_create_buffer_index() {
            auto type_index = detail::type_index::type_id<T>();
            auto it = m_buffer_indices.find(type_index);
            if (it == m_buffer_indices.end()) [[unlikely]] {
                std::size_t buffer_index = m_buffer_indices.size();
                m_buffer_indices.emplace(type_index, buffer_index);
                m_buffers.emplace_back(detail::make_buffer<std::decay_t<T>>());
                m_metabuffers.emplace_back();
                assert(m_buffer_indices.size() == m_buffers.size());
                assert(m_buffer_indices.size() == m_metabuffers.size());
                return buffer_index;
            }
            return it->second;
        }

        // Used in situations where we it is assumed that a buffer index exists.
        // i.e., when we clone an entity, remove components from an entity, 
        // check if an entity has components, retrieve components, query info
        // about component containers, remove unused space in component
        // containers, or retrieve existing views.
        template <typename T>
        [[nodiscard]] std::size_t get_buffer_index() const {
            auto type_index = detail::type_index::type_id<T>();
            assert(m_buffer_indices.contains(type_index));
            return m_buffer_indices.at(type_index);
        }

        using component_mask = boost::dynamic_bitset<>;

        [[nodiscard]] const detail::sparse_set<entity_type, traits_type>& 
        group_by(const std::initializer_list<std::size_t>& bids) {
            // Find the largest buffer index.
            std::size_t max_bid = std::max(bids);

            // Ensure we're only working with known components.
            assert(max_bid < m_buffer_indices.size());
            assert(max_bid < m_metabuffers.size());
            assert(max_bid < m_buffers.size());

            component_mask gid(max_bid + 1);
            for (auto bid : bids)
                gid.set(bid);

            auto it = m_groups.find(gid);
            if (it != m_groups.end()) [[likely]]
                return it->second;
            
            detail::sparse_set<entity_type, traits_type> group{};
            for (auto entity : m_entities) {
                auto eid = traits_type::index(entity);
                bool satisfied = true;
                for (auto bid : bids) {
                    const auto& metabuffer = m_metabuffers[bid];
                    if (eid >= metabuffer.size() || !metabuffer.test(eid)) {
                        satisfied = false;
                        break;
                    }
                }
                if (satisfied)
                    group.insert(entity);
            }
            it = m_groups.emplace_hint(it, gid, std::move(group));
            return it->second;
        }

        [[nodiscard]] const detail::sparse_set<entity_type, traits_type>&
        group_by(const std::initializer_list<std::size_t>& bids) const {
            // Find the largest buffer index.
            std::size_t max_bid = std::max(bids);

            // Ensure we're only working with known components.
            assert(max_bid < m_buffer_indices.size());
            assert(max_bid < m_metabuffers.size());
            assert(max_bid < m_buffers.size());

            component_mask gid(max_bid + 1);
            for (auto bid : bids)
                gid.set(bid);
            assert(m_groups.contains(gid));
            return m_groups.at(gid);
        }

        /**
         * Invariants:
         * 
         * 1) size of `m_inactive` is less than or equal to `m_entities`.
         * 2) sizes of `m_buffers`, `m_metabuffers`, and `m_buffer_indices` are
         *    the same.
         * 3) The size of any `component_mask` is less than or equal to the 
         *    sizes of `m_buffers`, `m_metabuffers`, and `m_buffer_indices`.
         */
        
        // Indices into `m_entities` denoting entities that can be reused.
        std::stack<std::size_t, boost::container::vector<std::size_t>> m_inactive{};

        // All entities, both active and inactive.
        boost::container::vector<entity_type> m_entities{};

        // Collection of type-erased pointers to component containers.
        boost::container::vector<detail::buffer_ptr> m_buffers{};

        // Meta data for each component container.
        boost::container::vector<boost::dynamic_bitset<>> m_metabuffers{};

        // Mapping from type to index within `m_buffers` and `m_metabuffers`.
        std::unordered_map<detail::type_index, std::size_t> m_buffer_indices{};

        // Groups of entities identified by a set of components.
        std::unordered_map<component_mask, detail::sparse_set<entity_type, traits_type>> m_groups{};

    };

    using world = basic_world<std::uint32_t>;

} // namespace ecfw