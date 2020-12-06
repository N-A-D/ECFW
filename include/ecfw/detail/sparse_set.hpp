#pragma once

#include <vector>   // vector
#include <memory>   // unique_ptr
#include <cassert>  // assert
#include <iterator> // reverse_iterator, random_access_iterator_tag
#include <boost/iterator/iterator_facade.hpp>
#include <ecfw/detail/entity.hpp>

namespace ecfw
{
namespace detail
{

    class sparse_set final {
    public:

        using difference_type = ptrdiff_t;
        using size_type       = size_t;
        using value_type      = uint64_t;
        using const_reference = const value_type&;
        using reference       = const_reference;
        using const_pointer   = const value_type*;
        using pointer         = const_pointer;
        
        struct iterator final
            : public boost::iterator_facade<
                iterator, 
                const value_type, 
                std::random_access_iterator_tag, 
                const_reference, 
                difference_type
            > 
        {
            iterator () = default;

            iterator(const_pointer it, const sparse_set& parent)
                : m_it{it}
                , m_parent{std::addressof(parent)}
            {}

        private:
            friend class boost::iterator_core_access;

            [[nodiscard]] const_reference dereference() const noexcept {
                return *m_it;
            }

            [[nodiscard]] bool equal(const iterator& other) const noexcept {
                assert(valid());
                assert(other.valid());
                return m_parent == other.m_parent && m_it == other.m_it;
            }

            void increment() noexcept {
                assert(valid());
                ++m_it;
            }

            void decrement() noexcept {
                assert(valid());
                --m_it;
            }

            void advance(difference_type n) noexcept {
                assert(valid());
                m_it += n;
            }

            [[nodiscard]] difference_type 
            distance_to(const iterator& other) const noexcept {
                assert(valid());
                assert(compatible_with(other));
                return other.m_it - m_it;
            }

            [[nodiscard]] bool valid() const noexcept {
                return m_parent != nullptr && m_it != nullptr;
            }

            [[nodiscard]] bool 
            compatible_with(const iterator& other) const noexcept {
                return m_parent == other.m_parent;
            }

            const_pointer m_it{nullptr};
            const sparse_set* m_parent{nullptr};
        };

        using const_iterator         = iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator       = const_reverse_iterator;

        sparse_set() = default;

        sparse_set(sparse_set&& rhs) noexcept
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

        [[nodiscard]] const_pointer data() const noexcept {
            return m_packed.data();
        }

        [[nodiscard]] const_iterator begin() const noexcept {
            return const_iterator(data(), *this);
        }

        [[nodiscard]] const_iterator end() const noexcept {
            return const_iterator(data() + size(), *this);
        }

        [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        [[nodiscard]] const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        void clear() noexcept {
            m_size = 0;
        }

        void insert(value_type val) {
            using std::make_unique;
            if (contains(val))
                return;

            // Get the block for this value
            size_type block = detail::index_from_entity(val) / block_size;

            // Get the block offset for this value
            size_type offset = detail::index_from_entity(val) & block_size - 1;

            // Ensure there is enough room in the sparse vector
            if (block >= m_sparse.size())
                m_sparse.resize(block + 1);

            // Blocks are allocated lazily to save on space
            if (!m_sparse[block])
                m_sparse[block] = make_unique<uint32_t[]>(block_size);
            
            // Ensure there is enough room in the packed vector
            if (m_size >= m_packed.size())
                m_packed.resize(m_size + 1);

            // Ensure the sparse vector can find the newly inserted value
            m_packed[m_size] = val;
            m_sparse[block][offset] = m_size;

            // Ensure the set's size is one-larger
            ++m_size;
        }

        void erase(value_type val) {
            using std::exchange;
            if (!contains(val))
                return;

            // Get the block for this value
            size_type block = detail::index_from_entity(val) / block_size;

            // Get the block offset for this value
            size_type offset = detail::index_from_entity(val) & block_size - 1;

            // Get the pointer to this value in the packed vector
            uint32_t index = m_sparse[block][offset];

            // Replaced the value to erase with the one at the back
            m_packed[index] = m_packed[m_size - 1];

            // Compute the block and offset for the moved value
            block = detail::index_from_entity(m_packed[index]) / block_size;
            offset = detail::index_from_entity(m_packed[index]) & block_size - 1;

            // Ensure the sparse vector can find the moved value
            m_sparse[block][offset] = index;

            // Ensure the set's size is one-smaller
            --m_size;
        }

        [[nodiscard]] bool contains(value_type val) const {
            size_type block = detail::index_from_entity(val) / block_size;
            size_type offset = detail::index_from_entity(val) & block_size - 1;
            // Check if the value's block exists, is allocated, contains a
            // pointer which fits within the packed vector, and matches the
            // stored value
            return block < m_sparse.size()
                && m_sparse[block]
                && m_sparse[block][offset] < m_size
                && m_packed[m_sparse[block][offset]] == val;
        }

        [[nodiscard]] size_type size() const noexcept {
            return m_size;
        }

        [[nodiscard]] bool empty() const noexcept {
            return m_size == 0;
        }

    private:

        static constexpr size_type block_size = 4096;

        // The number of elements stored in the set
        // The value may or may not be equivalent to 
        // the actualy size of the packed array
        uint32_t m_size{};

        // Collection of elements in the set
        std::vector<value_type> m_packed{};

        // Collection of arrays
        std::vector<std::unique_ptr<uint32_t[]>> m_sparse{};

    };

} // namespace detail
} // namespace ecfw
