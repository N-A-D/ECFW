#pragma once

#include <vector>   // vector
#include <memory>   // unique_ptr
#include <cassert>  // assert
#include <iterator> // reverse_iterator, random_access_iterator_tag
#include <ecfw/detail/integers.hpp>

namespace ecfw
{
namespace detail
{

	class sparse_set {
	public:

		using difference_type = ptrdiff_t;
		using size_type       = size_t;
		using value_type	  = uint64_t;
		using const_reference = const value_type&;
		using reference       = const_reference;
		using const_pointer   = const value_type*;
		using pointer         = const_pointer;
		
		class iterator {
		public:

			using iterator_category = std::random_access_iterator_tag;
			using difference_type   = sparse_set::difference_type;
			using value_type		= sparse_set::value_type;
			using reference			= sparse_set::reference;
			using pointer			= sparse_set::pointer;
			
			iterator() = default;
			
			iterator(pointer it, const sparse_set& parent)
				: m_it(it)
				, m_parent(std::addressof(parent))
			{}

			difference_type operator-(const iterator& rhs) const noexcept {
				assert(is_valid() && rhs.is_valid());
				assert(is_compatible_with(rhs));
				return m_it - rhs.m_it;
			}

			reference operator[](difference_type n) const {
				return *(*this + n);
			}

			pointer operator->() const {
				assert(is_valid());
				return m_it;
			}

			reference operator*() const {
				return *(operator->());
			}

			iterator& operator++() {
				assert(is_valid());
				++m_it;
				return *this;
			}

			iterator operator++(int) {
				iterator copy(*this);
				++* this;
				return copy;
			}

			iterator& operator+=(difference_type n) {
				assert(is_valid());
				m_it += n;
				return *this;
			}

			iterator operator+(difference_type n) const {
				return iterator(*this) += n;
			}

			iterator& operator--() {
				assert(is_valid());
				--m_it;
				return *this;
			}

			iterator operator--(int) {
				iterator copy(*this);
				--* this;
				return copy;
			}

			iterator& operator-=(difference_type n) {
				assert(is_valid());
				m_it -= n;
				return *this;
			}

			iterator operator-(difference_type n) const {
				return iterator(*this) -= n;
			}

			bool operator==(const iterator& rhs) const noexcept {
				assert(is_compatible_with(rhs));
				return m_it == rhs.m_it;
			}

			bool operator!=(const iterator& rhs) const noexcept {
				return !(*this == rhs);
			}

			bool operator<(const iterator& rhs) const noexcept {
				return m_it < rhs.m_it;
			}

			bool operator<=(const iterator& rhs) const noexcept {
				return *this < rhs || *this == rhs;
			}

			bool operator>(const iterator& rhs) const noexcept {
				return rhs < *this;
			}

			bool operator>=(const iterator& rhs) const noexcept {
				return *this > rhs || *this == rhs;
			}

		private:

			bool is_valid() const noexcept {
				return m_it && m_parent;
			}

			bool is_compatible_with(const iterator& rhs) const noexcept {
				return m_parent == rhs.m_parent;
			}

			pointer m_it{};
			const sparse_set* m_parent{};
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
			if (this != std::addressof(rhs)) {
				m_size = std::exchange(rhs.m_size, 0);
				m_packed = std::move(rhs.m_packed);
				m_sparse = std::move(rhs.m_sparse);
			}
			return *this;
		}

		const_pointer data() const noexcept {
			return m_packed.data();
		}

		const_iterator begin() const noexcept {
			return const_iterator(data(), *this);
		}

		const_iterator end() const noexcept {
			return const_iterator(data() + size(), *this);
		}

		const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		const_reverse_iterator rend() const noexcept {
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
			size_type block = lsw(val) / block_size;

			// Get the block offset for this value
			size_type offset = lsw(val) & block_size - 1;

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
			size_type block = lsw(val) / block_size;

			// Get the block offset for this value
			size_type offset = lsw(val) & block_size - 1;

			// Get the pointer to this value in the packed vector
			uint32_t index = m_sparse[block][offset];

			// Replaced the value to erase with the one at the back
			m_packed[index] = m_packed[m_size - 1];

			// Compute the block and offset for the moved value
			block = lsw(m_packed[index]) / block_size;
			offset = lsw(m_packed[index]) & block_size - 1;

			// Ensure the sparse vector can find the moved value
			m_sparse[block][offset] = index;

			// Ensure the set's size is one-smaller
			--m_size;
		}

		bool contains(value_type val) const {
			size_type block = lsw(val) / block_size;
			size_type offset = lsw(val) & block_size - 1;
			// Check if the value's block exists, is allocated, contains a
			// pointer which fits within the packed vector, and matches the
			// stored value
			return block < m_sparse.size()
				&& m_sparse[block]
				&& m_sparse[block][offset] < m_size
				&& m_packed[m_sparse[block][offset]] == val;
		}

		size_type size() const noexcept {
			return m_size;
		}

		bool empty() const noexcept {
			return m_size == 0;
		}

	private:

		static constexpr size_type block_size = 65536;

		// The number of elements stored in the set
		// The value may or may not be equivalent to 
		// the actualy size of the packed array
		uint32_t m_size{};

		// Collection of elements in the set
		std::vector<uint64_t> m_packed{};

		// Collection of arrays
		// Each array contains 2^16 entries which
		// serve as pointers into the packed vector
		// By storing arrays instead of actual values,
		// we can save on space
		std::vector<std::unique_ptr<uint32_t[]>> m_sparse{};

	};

} // namespace detail
} // namespace ecfw
