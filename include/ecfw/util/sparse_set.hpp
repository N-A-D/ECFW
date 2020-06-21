#pragma once

#include <vector>   // vector
#include <iterator>	// begin, data, size, reverse_iterator
#include <cassert>	// assert
#include <utility>	// exchange
#include <memory>	// std::unique_ptr
#include <ecfw/fwd/unsigned.hpp>
#include <ecfw/entity/entity.hpp>

namespace ecfw {

	namespace dtl = detail;

	/**
	 * @brief A representation of a sparse set of entities.
	 *
	 */
	class sparse_set final {
	public:

		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;
		using value_type      = u64;
		using reference       = const value_type&;
		using pointer         = const value_type*;

		/**
		 * @brief Random access sparse set iterator.
		 *
		 */
		class iterator final {
		public:

			using iterator_category = std::random_access_iterator_tag;
			using difference_type   = sparse_set::difference_type;
			using value_type        = sparse_set::value_type;
			using reference         = sparse_set::reference;
			using pointer           = sparse_set::pointer;

			/**
			 * @brief Default sparse_set::iterator construtor.
			 *
			 */
			iterator() = default;

			/**
			 * @brief Returns the distance between *this and another iterator.
			 *
			 * @param other: The other iterator.
			 * @return difference_type The distance between the two iterators.
			 */
			difference_type operator-(const iterator& other) const {
				assert(valid() && other.valid());
				assert(compatible(other));
				return m_it - other.m_it;
			}

			/**
			 * @brief Dereference operator.
			 *
			 * @return A reference to the pointed-to element.
			 */
			reference operator*() const {
				return *(operator->());
			}

			/**
			 * @brief Member access operator.
			 *
			 * @return A pointer to the pointed-to element.
			 */
			pointer operator->() const {
				assert(valid() && "Invalid iterator dereference!");
				return m_it;
			}

			/**
			 * @brief Prefix increment operator.
			 *
			 * @return *this incremented one position.
			 */
			iterator& operator++() {
				assert(valid() && "Invalid iterator increment!");
				++m_it;
				assert(m_parent->within_range(m_it));
				return *this;
			}

			/**
			 * @brief Posfix increment operator.
			 *
			 * @return An iterator to *this before the increment.
			 */
			iterator operator++(int) {
				iterator tmp(*this);
				++* this;
				return tmp;
			}

			/**
			 * @brief Increments *this n times.
			 *
			 * @param n: The value to increment by.
			 * @return *this
			 */
			iterator& operator+=(difference_type n) {
				assert(valid());
				m_it += n;
				assert(m_parent->within_range(m_it));
				return *this;
			}

			/**
			 * @brief Increment operator.
			 *
			 * @param n: The value to increment by.
			 * @return An iterator n positions higher than *this.
			 */
			iterator operator+(difference_type n) {
				return iterator(*this) += n;
			}

			/**
			 * @brief Prefix decrement operator.
			 *
			 * @return *this decremented one position.
			 */
			iterator& operator--() {
				assert(valid() && "Invalid iterator decrement!");
				--m_it;
				assert(m_parent->within_range(m_it));
				return *this;
			}

			/**
			 * @brief Postfix decrement operator.
			 *
			 * @return An iterator to *this before the decrement.
			 */
			iterator operator--(int) {
				iterator tmp(*this);
				--* this;
				return tmp;
			}

			/**
			 * @brief Decrements *this n times.
			 *
			 * @param n: The value to decrement by.
			 * @return iterator&
			 */
			iterator& operator-=(difference_type n) {
				return *this += -n;
			}

			/**
			 * @brief Decrement operator.
			 *
			 * @param n: The amount to decrement by.
			 * @return An iterator n positions lower than *this.
			 */
			iterator operator-(difference_type n) {
				return iterator(*this) -= n;
			}

			/**
			 * @brief Less-than comparison operator.
			 *
			 * @param other: The other iterator to compare to.
			 * @return true If *this is less than other, otherwise false.
			 */
			bool operator<(const iterator& other) const noexcept {
				assert(compatible(other) && "Iterators incompatible");
				return m_it < other.m_it;
			}

			/**
			 * @brief Less-than or equal operator.
			 *
			 * @param other: The other iterator to compare to.
			 * @return true If *this is lt or eq to other.
			 * @return false otherwise.
			 */
			bool operator<=(const iterator& other) const noexcept {
				return !(other < *this);
			}

			/**
			 * @brief Greater-than operator.
			 *
			 * @param other: The other iterator to compare to.
			 * @return true If *this is greater than other.
			 * @return false otherwise.
			 */
			bool operator>(const iterator& other) const noexcept {
				return other < *this;
			}

			/**
			 * @brief Greater-than or equal operator
			 *
			 * @param other: The other iterator to compare to.
			 * @return true If *this is gt or eq to other.
			 * @return false otherwisee.
			 */
			bool operator>=(const iterator& other) const noexcept {
				return !(*this < other);
			}

			/**
			 * @brief Equality operator.
			 *
			 * @param other: The other iterator to compare to.
			 * @return true If the two iterators are equivalent.
			 * @return false otherwise.
			 */
			bool operator==(const iterator& other) const noexcept {
				return !(*this < other) && !(other < *this);
			}

			/**
			 * @brief Inequality operator.
			 *
			 * @param other: The other iterator to compare to.
			 * @return true If the two iterators are not equal.
			 * @return false otherwise.
			 */
			bool operator!=(const iterator& other) const noexcept {
				return !(*this == other);
			}

		private:

			friend class sparse_set;

			/**
			 * @brief Constructs a valid sparse_set::iterator
			 *
			 * @param parent A pointer to the sparse set which created *this.
			 * @param it A pointer to the current element.
			 */
			iterator(const sparse_set* parent, pointer it)
				: m_parent(parent)
				, m_it(it)
			{}

			const sparse_set* m_parent{};
			pointer m_it{};

			/**
			 * @brief Checks if *this is okay to use.
			 *
			 * @return true If valid, otherwise false.
			 */
			bool valid() const noexcept {
				return m_parent != nullptr && m_it != nullptr;
			}

			/**
			 * @brief Checks if two iterators were made from the same
			 * sparse set.
			 *
			 * @param other: The other iterator to test.
			 * @return true If both iterators come from the same sparse set, otherwise false.
			 */
			bool compatible(const iterator& other) const noexcept {
				return  m_parent == other.m_parent;
			}
		};

		using reverse_iterator = std::reverse_iterator<iterator>;

		/**
		 * @brief Default ctor.
		 *
		 */
		sparse_set() = default;

		/**
		 * @brief Default destructor.
		 *
		 */
		~sparse_set() = default;

		/**
		 * @brief Move constructor.
		 *
		 * @param other: The container to move from.
		 */
		sparse_set(sparse_set&& other)
			: m_size{ std::exchange(other.m_size, 0) }
			, m_packed{ std::move(other.m_packed) }
			, m_sparse{ std::move(other.m_sparse) }
		{}

		/**
		 * @brief Move assignment operator.
		 *
		 * @param other: The other container to move from.
		 * @return *this.
		 */
		sparse_set& operator=(sparse_set&& other) noexcept {
			if (std::addressof(other) != this) {
				m_packed = std::move(other.m_packed);
				m_sparse = std::move(other.m_sparse);
				m_size = std::exchange(other.m_size, 0);
			}
			return *this;
		}

		/**
		 * @brief Returns a pointer to the packed integer vector.
		 *
		 * @note [data(), data() + size()) is always a valid range.
		 *
		 * @return A pointer to the packed integer vector.
		 */
		pointer data() const noexcept {
			return std::data(m_packed);
		}

		/**
		 * @brief Returns an iterator to the beginning of the set.
		 *
		 * @return An iterator to the beginning of the set.
		 */
		iterator begin() const noexcept {
			return { this, data() };
		}

		/**
		 * @brief Returns an iterator to one past the end of the set.
		 *
		 * @return An iterator to one past the end of the set.
		 */
		iterator end() const noexcept {
			return { this, data() + size() };
		}

		/**
		 * @brief 
		 * Returns a reverse_iterator to the first element of the reversed set.
		 *
		 * @return A reverse_iterator to the first element of the reversed set.
		 */
		reverse_iterator rbegin() const noexcept {
			return reverse_iterator{ end() };
		}

		/**
		 * @brief 
		 * Returns a reverse_iterator to one-past the end of the reversed set.
		 *
		 * @return A reverse_iterator to one past the end of the reversed set.
		 */
		reverse_iterator rend() const noexcept {
			return reverse_iterator{ begin() };
		}

		/**
		 * @brief Checks if the set is empty.
		 *
		 * @return true If the set does not have any elements, false otherwise.
		 */
		bool empty() const noexcept {
			return size() == 0;
		}

		/**
		 * @brief Returns the size of the set.
		 *
		 * @return The number of elements in the set.
		 */
		size_type size() const noexcept {
			return m_size;
		}

		/**
		 * @brief Clears the contents of the set.
		 *
		 */
		void clear() noexcept {
			m_size = 0;
		}

		/**
		 * @brief Adds a new element to the set.
		 *
		 * @note No-op if the element already exists within the set.
		 *
		 * @param val: The value to insert.
		 */
		void insert(value_type val) {
			if (!contains(val)) {
				auto [_, i] = dtl::unpack_entity(val);

				// Resize the redirection table to accommodate
				// the candidate's index if necessary
				if (i >= std::size(m_sparse) * block_size)
					m_sparse.resize(block(i) + 1);

				// Resize the packed vector if it is at capacity
				if (m_size >= std::size(m_packed))
					m_packed.resize(m_size + 1);

				// Ensure the block which associates with the
				// given candidate's index is initialized
				ensure_memory_at(i);

				// Ensure the sparse vector points to the
				// index of the new entity
				m_sparse[block(i)][offset(i)] = static_cast<u32>(m_size);

				// Insert the new entity into the position
				// the sparse vector is told it's in
				m_packed[m_size] = val;

				// Increment the number of stored entities
				++m_size;
			}
		}

		/**
		 * @brief Removes an element from the set.
		 *
		 * @note No-op if the element is not found in the set.
		 *
		 * @param val: The value to erase.
		 */
		void erase(value_type val) {
			if (contains(val)) {
				// Get the index portion of the entity to be removed
				auto [v_old, i_old] = dtl::unpack_entity(val);

				// Get the index portion of the entity at the end
				auto [v_new, i_new] = 
						dtl::unpack_entity(m_packed[m_size - 1]);

				// Swap the entity to be removed with the one at the end
				m_packed[m_sparse[block(i_old)][offset(i_old)]] = 
						m_packed[m_size - 1];

				// Ensure lookup for the moved entity succeeds
				m_sparse[block(i_new)][offset(i_new)] = 
						m_sparse[block(i_old)][offset(i_old)];

				// Decrement the number of stored entities
				--m_size;
			}
		}

		/**
		 * @brief Determines if an element is a part of the set.
		 *
		 * @param val: The element to search for.
		 * @return true If the element is found in the set, false otherwise.
		 */
		bool contains(value_type val) const {
			auto [_, i] = dtl::unpack_entity(val);
			return i < std::size(m_sparse)* block_size
				&& m_sparse[block(i)]
				&& m_sparse[block(i)][offset(i)] < size()
				&& m_packed[m_sparse[block(i)][offset(i)]] == val;
		}

	private:

		// The number of indices per block
		static constexpr std::size_t block_size = 32768;

		/**
		 * @brief Determines if a pointer fits within the sets range.
		 *
		 * @param p: The pointer to test.
		 * @return true If the pointer is within range, false otherwise.
		 */
		bool within_range(pointer p) const noexcept {
			return data() <= p && p <= data() + size();
		}

		/**
		 * @brief Ensures the memory at  the given index is allocated.
		 *
		 * @param pos: The index to accommodate.
		 */
		void ensure_memory_at(size_type pos) {
			using std::make_unique;

			size_type block_n = block(pos);
			if (pos >= std::size(m_sparse) * block_size)
				m_sparse.resize(block_n + 1);
			if (!m_sparse[block_n])
				m_sparse[block_n] =
				make_unique<u32[]>(block_size);
		}

		/**
		 * @brief Returns an index to a memory block.
		 *
		 * @param pos: An index to map to a memory block.
		 */
		size_type block(size_type pos) const noexcept {
			return pos / block_size;
		}

		/**
		 * @brief Returns a memory block offset.
		 *
		 * @param pos: The index to offset.
		 */
		size_type offset(size_type pos) const noexcept {
			return (pos & block_size - 1);
		}

		// The number of elements stored
		size_type m_size{};

		// Unordered collection of integers
		std::vector<value_type> m_packed{};

		// Redirection table. Entries are indices into m_packed
		std::vector<std::unique_ptr<u32[]>> m_sparse{};
	};
}