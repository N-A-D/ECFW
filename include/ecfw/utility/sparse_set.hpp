#pragma once

#include <vector>   // std::vector
#include <iterator> // std::bidirectional_iterator_tag, std::data
#include <cassert>  // assert
#include <utility>  // std::exchange
#include <initializer_list> // std::initializer_list
#include <ecfw/fwd/unsigned.hpp>

namespace ecfw {
	namespace detail {

		/**
		 * @brief Sparse integer set
		 * 
		 * A custom implementation of a sparse set.
		 * 
		 * Provides O(1) lookup + erase, amortized O(1) insert.
		 * 
		 */
		class sparse_set {
		public:

			using container_type  = std::vector<u32>;
			using size_type       = u32;
			using difference_type = typename container_type::difference_type;
			using value_type      = typename container_type::value_type;
			using reference       = typename container_type::const_reference;
			using pointer         = typename container_type::const_pointer;
			
			/**
			 * @brief Bidirectional sparse set iterator.
			 * 
			 */
			class iterator {	
			public:

				using iterator_category = std::bidirectional_iterator_tag;
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
					++*this;
					return tmp;
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
					--*this;
					return tmp;
				}

				/**
				 * @brief Equality operator.
				 * 
				 * @param other The other iterator to compare to.
				 * @return true If the two iterators are equivalent.
				 * @return false otherwise.
				 */
				bool operator==(const iterator& other) const noexcept {
					assert(valid());
					assert(compatible(other));
					return m_it == other.m_it;
				}

				/**
				 * @brief Inequality operator.
				 * 
				 * @param other The other iterator to compare to.
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

				const sparse_set* m_parent{ nullptr };
				pointer m_it{ nullptr };

				/**
				 * @brief Checks if *this is okay to use.
				 * 
				 * @return true If valid.
				 * @return false otherwise. 
				 */
				bool valid() const noexcept {
					return m_parent != nullptr && m_it != nullptr;
				}

				/**
				 * @brief Checks if two iterators were made from the same
				 * sparse set.
				 * 
				 * @param other The other iterator to test.
				 * @return true If both iterators come from the same sparse set.
				 * @return false otherwise.
				 */
				bool compatible(const iterator& other) const noexcept {
					return  m_parent == other.m_parent;
				}
			};

			/**
			 * @brief Default ctor.
			 * 
			 */
			sparse_set() = default;

			/**
			 * @brief Default dtor.
			 * 
			 */
			~sparse_set() = default;

			/**
			 * @brief Default copy ctor.
			 * 
			 */
			sparse_set(const sparse_set&) = default;

			/**
			 * @brief Default copy assignment operator.
			 * 
			 * @return *this.
			 */
			sparse_set& operator=(const sparse_set&) = default;

			/**
			 * @brief Move constructor.
			 * 
			 * @param other The container to move from.
			 */
			sparse_set(sparse_set&& other)
				: m_size{ std::exchange(other.m_size, 0) }
				, m_packed{std::move(other.m_packed)}
				, m_sparse{std::move(other.m_sparse)}
			{}

			/**
			 * @brief Move assignment operator.
			 * 
			 * @param other The other container to move from.
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

			using reverse_iterator = std::reverse_iterator<iterator>;

			/**
			 * @brief Returns a pointer to the packed integer vector.
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
				return iterator(this, data());
			}

			/**
			 * @brief Returns an iterator to one past the end of the set.
			 * 
			 * @return An iterator to one past the end of the set.
			 */
			iterator end() const noexcept {
				return iterator(this, data() + size());
			}

			/**
			 * @brief Returns a reverse iterator to the first element of the reversed set.
			 * 
			 * @return A reverse iterator to the first element of the reversed set.
			 */
			reverse_iterator rbegin() const noexcept {
				return reverse_iterator(end());
			}

			/**
			 * @brief Returns a reverse iterator to one past the end of the reversed set.
			 * 
			 * @return A reverse iterator to one past the end of the reversed set.
			 */
			reverse_iterator rend() const noexcept {
				return reverse_iterator(begin());
			}

			/**
			 * @brief Checks if the set is empty.
			 * 
			 * @return true If the set does not have any elements.
			 * @return false otherwise.
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
			 * @param val The value to insert.
			 */
			void insert(value_type val) {
				if (!contains(val)) {
					if (val >= std::size(m_sparse))
						m_sparse.resize(val + 1);
					if (m_size >= std::size(m_packed))
						m_packed.resize(m_size + 1);
					m_sparse[val] = m_size;
					m_packed[m_size] = val;
					++m_size;
				}
			}

			/**
			 * @brief Removes an element from the set.
			 * 
			 * @note No-op if the element is not found in the set.
			 * 
			 * @param val The value to erase.
			 */
			void erase(value_type val) {
				if (contains(val)) {
					m_packed[m_sparse[val]] = m_packed[m_size - 1];
					m_sparse[m_packed[m_size - 1]] = m_sparse[val];
					--m_size;
				}
			}

			/**
			 * @brief Determines if an element is a part of the set.
			 *
			 * @param val The element to search for.
			 * @return true If the element is found in the set.
			 * @return false otherwise.
			 */
			bool contains(value_type val) const {
				return val < std::size(m_sparse)
					&& m_sparse[val] < size()
					&& m_packed[m_sparse[val]] == val;
			}

		private:

			/**
			 * @brief Determines if a pointer fits within the sets range.
			 * 
			 * @param p The pointer to test.
			 * @return true If the pointer is within range.
			 * @return false otherwise.
			 */
			bool within_range(pointer p) const noexcept {
				return data() <= p && p <= data() + size();
			}

			size_type m_size{};
			container_type m_packed{};
			container_type m_sparse{};
		};

	}
}
