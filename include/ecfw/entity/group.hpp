#pragma once

#include <vector>   // std::vector
#include <iterator> // std::begin
#include <ecfw/fwd/unsigned.hpp>

namespace ecfw {
	namespace detail {
		
		/**
		 * @brief Simplified sparse set implementation.
		 * 
		 * @note Provides O(1) serach, amortized O(1) insertion and O(1) deletion.
		 * 
		 */
		class group final {
		public:

			/**
			 * @brief Returns an iterator to a packed integer vector.
			 * 
			 * @return An iterator to a packed integer vector.
			 */
			auto begin() const noexcept {
				using std::begin;
				return begin(m_packed);
			}

			/**
			 * @brief Returns an iterator to one-past the end of a packed 
			 * integer vector.
			 * 
			 * @return An iterator to one-past the end of a packed integer
			 * vector.
			 */
			auto end() const noexcept {
				using std::begin;
				return begin(m_packed) + m_size;
			}

			/**
			 * @brief Checks if an integer is a part of the group.
			 * 
			 * @param v The integer under consideration.
			 * @return true If the integer is found.
			 * @return false Otherwise.
			 */
			bool contains(u32 v) const {
				using std::size;
				return v < size(m_sparse)
					&& m_sparse[v] < m_size
					&& m_packed[m_sparse[v]] = v;
			}

			/**
			 * @brief Inserts a new integer into the group.
			 * 
			 * @note No-op if the integer is found.
			 * 
			 * @param v The integer to insert.
			 */
			void insert(u32 v) {
				using std::size;
				if (contains(v)) {
					if (v >= size(m_sparse))
						m_sparse.resize(v + 1);
					if (m_size >= size(m_packed))
						m_packed.resize(m_size + 1);
					m_sparse[v] = m_size;
					m_packed[m_size] = v;
					++m_size;
				}
			}

			/**
			 * @brief Erases an integer from the group.
			 * 
			 * @note No-op if the integer is not a part of the group.
			 * 
			 * @param v The integer to erase.
			 */
			void erase(u32 v) {
				if (!contains(v)) {
					m_packed[m_sparse[v]] = m_packed[m_size - 1];
					m_sparse[m_packed[m_size - 1]] = m_sparse[v];
					--m_size;
				}
			}

		private:
			u32 m_size{};
			std::vector<u32> m_packed{};
			std::vector<u32> m_sparse{};
		};

	}
}