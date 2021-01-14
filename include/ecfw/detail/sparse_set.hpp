#pragma once

#include <limits>
#include <memory>
#include <cassert>
#include <concepts>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/container/vector.hpp>
#include <ecfw/entity.hpp>

namespace ecfw
{
namespace detail
{

    template <Entity E, typename Traits = entity_traits<E>>
        requires std::same_as<E, typename Traits::entity_type>
    class sparse_set final {
    public:
        using traits_type = entity_traits<E>;
        using entity_type = typename traits_type::entity_type;

        static constexpr std::size_t block_size = 
            std::numeric_limits<entity_type>::digits 
            * std::numeric_limits<entity_type>::digits;

        class iterator final : public boost::iterator_facade<
            iterator, const entity_type, boost::random_access_traversal_tag> {
        public:
            using base_type = boost::iterator_facade<
                iterator, const entity_type, boost::random_access_traversal_tag>;

            using iterator_category = typename base_type::iterator_category;
            using difference_type   = typename base_type::difference_type;
            using value_type        = typename base_type::value_type;
            using reference         = typename base_type::reference;
            using pointer           = typename base_type::pointer;

            iterator() = default;

        private:
            [[nodiscard]] bool is_valid() const noexcept {
                return m_parent && m_it;
            }

            [[nodiscard]] bool 
            is_compatible_with(const iterator& rhs) const noexcept {
                return m_parent == rhs.m_parent;
            }

            friend class boost::iterator_core_access;

            [[nodiscard]] const entity_type& dereference() const noexcept {
                assert(is_valid());
                return *m_it;
            }

            [[nodiscard]] bool equal(const iterator& rhs) const noexcept {
                assert(is_valid());
                assert(rhs.is_valid());
                return m_parent == rhs.m_parent && m_it == rhs.m_it;
            }

            void increment() noexcept {
                assert(is_valid());
                ++m_it;
            }

            void decrement() noexcept {
                assert(is_valid());
                --m_it;
            }

            void advance(difference_type n) noexcept {
                assert(is_valid());
                m_it += n;
            }

            [[nodiscard]] difference_type 
            distance_to(const iterator& other) const noexcept {
                assert(is_valid());
                assert(is_compatible_with(other));
                return other.m_it - m_it;
            }

            friend class sparse_set<E, Traits>;

            iterator(const entity_type* it, const sparse_set<E, Traits>* parent)
                : m_it(it)
                , m_parent(parent)
            {}

            const entity_type* m_it{};
            const sparse_set<E, Traits>* m_parent{};
        };

        using reverse_iterator = std::reverse_iterator<iterator>;

        sparse_set() = default;

        sparse_set(sparse_set&& rhs)
            : m_size(std::exchange(rhs.m_size, 0))
            , m_packed(std::move(rhs.m_packed))
            , m_sparse(std::move(rhs.m_sparse))
        {}

        sparse_set& operator=(sparse_set&& rhs) noexcept {
            if (this != std::addressof(rhs)) [[likely]] {
                m_size = std::exchange(rhs.m_size, 0);
                m_packed = std::move(rhs.m_packed);
                m_sparse = std::move(rhs.m_sparse);
            }
            return *this;
        }

        [[nodiscard]] iterator begin() const noexcept {
            return iterator{ data(), this };
        }

        [[nodiscard]] iterator end() const noexcept {
            return iterator{ data() + size(), this };
        }

        [[nodiscard]] reverse_iterator rbegin() const noexcept {
            return reverse_iterator{ end() };
        }

        [[nodiscard]] reverse_iterator rend() const noexcept {
            return reverse_iterator{ begin() };
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] bool empty() const noexcept {
            return m_size == 0;
        }

        void clear() noexcept {
            m_size = 0;
        }

        [[nodiscard]] bool contains(entity_type e) const {
            std:size_t block = traits_type::index(e) / block_size;
            std::size_t offset = traits_type::index(e) & block_size - 1;
            return block < m_sparse.size()
                && m_sparse[block]
                && m_sparse[block][offset] < m_size
                && m_packed[m_sparse[block][offset]] == e;
        }

        void insert(entity_type e) {
            if (contains(e))
                return;
            std::size_t block = traits_type::index(e) / block_size;
            std::size_t offset = traits_type::index(e) & block_size - 1;
            if (block >= m_sparse.size())
                m_sparse.resize(block + 1);
            if (!m_sparse[block])
                m_sparse[block] = std::make_unique<std::size_t[]>(block_size);
            if (m_size >= m_packed.size())
                m_packed.resize(m_size + 1);
            m_packed[m_size] = e;
            m_sparse[block][offset] = m_size;
            ++m_size;
        }

        void erase(entity_type e) {
            if (!contains(e))
                return;
            std::size_t block = traits_type::index(e) / block_size;
            std::size_t offset = traits_type::index(e) & block_size - 1;
            auto idx = m_sparse[block][offset];
            m_packed[idx] = m_packed[m_size - 1];
            block = traits_type::index(m_packed[idx]) / block_size;
            offset = traits_type::index(m_packed[idx]) & block_size - 1;
            m_sparse[block][offset] = idx;
            --m_size;
        }

    private:

        [[nodiscard]] const entity_type* data() const noexcept {
            return m_packed.data();
        }

        std::size_t m_size{};
        boost::container::vector<entity_type> m_packed{};
        boost::container::vector<std::unique_ptr<std::size_t[]>> m_sparse{};
    };



} // namespace detail
} // namespace ecfw
