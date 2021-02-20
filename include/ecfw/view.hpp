#pragma once

#include <concepts>
#include <type_traits>
#include <ecfw/entity.hpp>
#include <ecfw/detail/buffer.hpp>
#include <ecfw/detail/sparse_set.hpp>
#include <ecfw/detail/type_list.hpp>

namespace ecfw
{

    template <Entity E, typename Traits>
        requires std::same_as<E, typename Traits::entity_type>
    class basic_world;

    template <Entity E, typename Traits, typename... Ts>
        requires std::same_as<E, typename Traits::entity_type>
    class basic_view final {

        static constexpr auto viewed = detail::type_list_v<Ts...>;

        template <typename T>
        using buffer_type = std::conditional_t<
            std::is_const_v<T>, const detail::buffer<T>, detail::buffer<T>>;

    public:
        using traits_type = Traits;
        using entity_type = typename Traits::entity_type;

        using iterator = 
            typename detail::sparse_set<entity_type, traits_type>::iterator;
        using reverse_iterator = 
            typename detail::sparse_set<entity_type, traits_type>::reverse_iterator;

        /**
         * @brief Default move constructor.
         * 
         */
        basic_view(basic_view&&) = default;

        /**
         * @brief Default copy constructor.
         * 
         */
        basic_view(const basic_view&) = default;

        /**
         * @brief Default move asignment operator.
         * 
         * @return *this.
         */
        basic_view& operator=(basic_view&&) = default;

        /**
         * @brief Default copy assignment operator.
         * 
         * @return *this.
         */
        basic_view& operator=(const basic_view&) = default;

        /**
         * @brief Returns an iterator to the first entity in the view.
         * 
         * @return An iterator to the first entity in the view.
         */
        [[nodiscard]] iterator begin() const noexcept {
            return m_entities->begin();
        }

        /**
         * @brief Returns an iterator to one-past the last entity in the view.
         * 
         * @return An iterator to one-past the last entity in the view.
         */
        [[nodiscard]] iterator end() const noexcept {
            return m_entities->end();
        }

        /**
         * @brief Returns a reverse_iterator to the first entity of the 
         * reversed view.
         * 
         * @return A reverse_iterator to the first entity.
         */
        [[nodiscard]] reverse_iterator rbegin() const noexcept {
            return m_entities->rbegin();
        }

        /**
         * @brief Returns a reverse_iterator to one-past the last entity in 
         * the reversed view.
         * 
         * @return A reverse_iterator to one-past the last entity.
         */
        [[nodiscard]] reverse_iterator rend() const noexcept {
            return m_entities->rend();
        }

        /**
         * @brief Returns the number of entities viewed by *this.
         * 
         * @return The number of entities viewed.
         */
        [[nodiscard]] std::size_t size() const noexcept {
            return m_entities->size();
        }

        /**
         * @brief Checks if there are any entities viewed by *this.
         * 
         * @return true If there are no entities viewed by *this.
         * @return false If there is at least one entity viewed by *this.
         */
        [[nodiscard]] bool empty() const noexcept {
            return m_entities->empty();
        }

        /**
         * @brief Checks if an entity is viewed by *this.
         * 
         * @param e The entity to search for.
         * @return true If the entity is viewed by *this.
         * @return false If the entity is not viewed by *this.
         */
        [[nodiscard]] bool contains(entity_type e) const {
            return m_entities->contains(e);
        }

        /**
         * @brief Gets an entity's component(s).
         * 
         * @pre The entity is viewed by *this.
         * 
         * @tparam T The first component to obtain.
         * @tparam Ts The other component(s) to obtain.
         * @param e The owning entity.
         * @return A reference or tuple of references.
         */
        template <typename... Cs>
        [[nodiscard]] decltype(auto) get(entity_type e) const {
            assert(contains(e));
            return unchecked_get<Cs...>(traits_type::index(e));
        }

    private:
        friend class basic_world<entity_type, traits_type>;
        
        template <typename... Cs>
        [[nodiscard]] decltype(auto) 
        unchecked_get(typename traits_type::index_type idx) const {
            using detail::index_of;
            using detail::type_v;

            if constexpr (sizeof...(Cs) == 1) {
                auto buffer = (m_buffers[index_of(type_v<Cs>, viewed)], ...);
                return (*buffer)[idx];
            }
            else if constexpr (sizeof...(Cs) > 1)
                return std::forward_as_tuple(unchecked_get<Cs>(idx)...);
            else 
                return std::forward_as_tuple(unchecked_get<Ts>(idx)...);
        }

        basic_view(const detail::sparse_set<entity_type, traits_type>& entities, 
                buffer_type<Ts>&... buffers)
            : m_buffers(std::addressof(buffers)...)
            , m_entities(std::addressof(entities))
        {}

        boost::hana::tuple<buffer_type<Ts>*...> m_buffers{};
        const detail::sparse_set<entity_type, traits_type>* m_entities{};
    };

    template <Entity E, typename Traits, typename T>
        requires std::same_as<E, typename Traits::entity_type>
    class basic_view<E, Traits, T> final {
        
        template <typename C>
        using buffer_type = std::conditional_t<
            std::is_const_v<C>, const detail::buffer<C>, detail::buffer<C>>;

    public:
        using traits_type = Traits;
        using entity_type = typename Traits::entity_type;

        using iterator = 
            typename detail::sparse_set<entity_type, traits_type>::iterator;
        using reverse_iterator = 
            typename detail::sparse_set<entity_type, traits_type>::reverse_iterator;

        /**
         * @brief Default move constructor.
         * 
         */
        basic_view(basic_view&&) = default;

        /**
         * @brief Default copy constructor.
         * 
         */
        basic_view(const basic_view&) = default;

        /**
         * @brief Default move asignment operator.
         * 
         * @return *this.
         */
        basic_view& operator=(basic_view&&) = default;

        /**
         * @brief Default copy assignment operator.
         * 
         * @return *this.
         */
        basic_view& operator=(const basic_view&) = default;

        /**
         * @brief Returns an iterator to the first entity in the view.
         * 
         * @return An iterator to the first entity in the view.
         */
        [[nodiscard]] iterator begin() const noexcept {
            return m_entities->begin();
        }

        /**
         * @brief Returns an iterator to one-past the last entity in the view.
         * 
         * @return An iterator to one-past the last entity in the view.
         */
        [[nodiscard]] iterator end() const noexcept {
            return m_entities->end();
        }

        /**
         * @brief Returns a reverse_iterator to the first entity of the 
         * reversed view.
         * 
         * @return A reverse_iterator to the first entity.
         */
        [[nodiscard]] reverse_iterator rbegin() const noexcept {
            return m_entities->rbegin();
        }

        /**
         * @brief Returns a reverse_iterator to one-past the last entity in 
         * the reversed view.
         * 
         * @return A reverse_iterator to one-past the last entity.
         */
        [[nodiscard]] reverse_iterator rend() const noexcept {
            return m_entities->rend();
        }

        /**
         * @brief Returns the number of entities viewed by *this.
         * 
         * @return The number of entities viewed.
         */
        [[nodiscard]] std::size_t size() const noexcept {
            return m_entities->size();
        }

        /**
         * @brief Checks if there are any entities viewed by *this.
         * 
         * @return true If there are no entities viewed by *this.
         * @return false If there is at least one entity viewed by *this.
         */
        [[nodiscard]] bool empty() const noexcept {
            return m_entities->empty();
        }

        /**
         * @brief Checks if an entity is viewed by *this.
         * 
         * @param e The entity to search for.
         * @return true If the entity is viewed by *this.
         * @return false If the entity is not viewed by *this.
         */
        [[nodiscard]] bool contains(entity_type e) const {
            return m_entities->contains(e);
        }

        /**
         * @brief Returns an entity's component.
         * 
         * @pre The entity is viewed by *this.
         * 
         * @param e The owning entity.
         * @return A reference to the entity's component.
         */
        [[nodiscard]] decltype(auto) get(entity_type e) const {
            assert(contains(e));
            return (*m_buffer)[traits_type::index(e)];
        }

    private:
        friend class basic_world<entity_type, traits_type>;

        basic_view(const detail::sparse_set<entity_type, traits_type>& entities,
                buffer_type<T>& buffer)
            : m_buffer(std::addressof(buffer))
            , m_entities(std::addressof(entities))
        {}

        buffer_type<T>* m_buffer{};
        const detail::sparse_set<entity_type, traits_type>* m_entities{};
    };

    template <typename... Ts>
    using view = basic_view<std::uint32_t, entity_traits<std::uint32_t>, Ts...>;

} // namespace ecfw