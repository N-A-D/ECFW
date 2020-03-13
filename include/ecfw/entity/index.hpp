#pragma once

#include <vector>        // vector
#include <iterator>		 // begin, data, size, reverse_iterator
#include <cassert>		 // assert
#include <cstdint>		 // uint32_t, uint64_t
#include <utility>		 // exchange
#include <unordered_map> // unordered_map
#include <boost/hana.hpp>
#include <bitset2/bitset2.hpp>
#include <ecfw/fwd/unsigned.hpp>

namespace ecfw { namespace detail {

	namespace bs2  = Bitset2;
	namespace hana = boost::hana;

	template <std::size_t N>
	using bitmask = bs2::bitset2<N>;

	/**
	 * @brief Determines the index to an element in an iterable.
	 *
	 * @note Taken from: https://stackoverflow.com/questions/33979592/boost-hana-get-index-of-first-matching
	 *
	 * @tparam T The element type.
	 * @tparam Iterable The iterable sequence type.
	 * @param element An element to search for.
	 * @param iterable An iterable sequence to search in.
	 * @return An index to the first occurence of the element. Otherwise,
	 * the length of the iterable sequence.
	 */
	template <typename T, typename Iterable>
	constexpr auto index_of(const T& element, const Iterable& iterable) {
		auto size = decltype(hana::size(iterable)){};
		auto dropped = decltype(hana::size(
			hana::drop_while(iterable, hana::not_equal.to(element))
		)){};
		return size - dropped;
	}
	
	/**
	 * @brief Encodes a subset of a larger sequence as a bitmask.
	 * 
	 * The purpose of this function is to provide compile-time 
	 * bitmasks that can be used to index entities based on the
	 * components they have.
	 * 
	 * @tparam Xs An iterable sequence type.
	 * @tparam Ys An iterable sequence type.
	 * @param xs A sequence of types that is a subset of ys.
	 * @param ys A sequence of types that is a supset set of xs.
	 * @return A bitmask whose active bits represent elements of the subset.
	 */
	template <typename Xs, typename Ys>
	constexpr auto encode_subset(const Xs& xs, const Ys& ys) {
		static_assert(hana::is_subset(decltype(xs){}, decltype(ys){}));
		auto size = decltype(hana::size(ys)){};
		bitmask<size> mask;
		auto encode = [&](auto t) { mask.set(index_of(t, ys)); };
		hana::for_each(xs, encode);
		return mask;
	}
	
	/**
	 * @brief Sparse integer set
	 *
	 * A custom implementation of a sparse set.
	 *
	 * Provides O(1) lookup + erase, amortized O(1) insert.
	 *
	 */
	class sparse_set final {
	public:

		using container_type  = std::vector<u32>;
		using size_type		  = u32;
		using difference_type = typename container_type::difference_type;
		using value_type	  = typename container_type::value_type;
		using reference		  = typename container_type::const_reference;
		using pointer		  = typename container_type::const_pointer;

		/**
		 * @brief Random access sparse set iterator.
		 *
		 */
		class iterator final {
		public:

			using iterator_category = std::random_access_iterator_tag;
			using difference_type	= sparse_set::difference_type;
			using value_type		= sparse_set::value_type;
			using reference			= sparse_set::reference;
			using pointer			= sparse_set::pointer;

			/**
			 * @brief Default sparse_set::iterator construtor.
			 *
			 */
			iterator() = default;

			/**
			 * @brief Returns the distance between *this and another iterator.
			 *
			 * @param other The other iterator.
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
			 * @note Complexity: O(1).
			 *
			 * @param n The value to increment by.
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
			 * @param n The value to increment by.
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
			 * @note Complexity: O(1).
			 *
			 * @param n The value to decrement by.
			 * @return iterator&
			 */
			iterator& operator-=(difference_type n) {
				return *this += -n;
			}

			/**
			 * @brief Decrement operator.
			 *
			 * @param n The amount to decrement by.
			 * @return An iterator n positions lower than *this.
			 */
			iterator operator-(difference_type n) {
				return iterator(*this) -= n;
			}

			/**
			 * @brief Less-than comparison operator.
			 *
			 * @param other The other iterator to compare to.
			 * @return true If *this is less than other.
			 * @return false otherwise.
			 */
			bool operator<(const iterator& other) const noexcept {
				assert(compatible(other) && "Iterators incompatible");
				return m_it < other.m_it;
			}

			/**
			 * @brief Less-than or equal operator.
			 *
			 * @param other The other iterator to compare to.
			 * @return true If *this is lt or eq to other.
			 * @return false otherwise.
			 */
			bool operator<=(const iterator& other) const noexcept {
				return !(other < *this);
			}

			/**
			 * @brief Greater-than operator.
			 *
			 * @param other The other iterator to compare to.
			 * @return true If *this is greater than other.
			 * @return false otherwise.
			 */
			bool operator>(const iterator& other) const noexcept {
				return other < *this;
			}

			/**
			 * @brief Greater-than or equal operator
			 *
			 * @param other The other iterator to compare to.
			 * @return true If *this is gt or eq to other.
			 * @return false otherwisee.
			 */
			bool operator>=(const iterator& other) const noexcept {
				return !(*this < other);
			}

			/**
			 * @brief Equality operator.
			 *
			 * @param other The other iterator to compare to.
			 * @return true If the two iterators are equivalent.
			 * @return false otherwise.
		 	 */
			bool operator==(const iterator& other) const noexcept {
				return !(*this < other) && !(other < *this);
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
		 * @brief Default copy constructor.
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
			: m_size{std::exchange(other.m_size, 0)}
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

	template <typename Bitmask, typename ResultSet>
	using index_map = std::unordered_map<Bitmask, ResultSet>;

} }