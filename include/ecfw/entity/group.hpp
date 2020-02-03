#pragma once

#include <vector>
#include <iterator> // std::begin
#include <ecfw/fwd/unsigned.hpp>

namespace ecfw {
	namespace detail {
		
		/**
		 * @brief Simplified sparse set implementation.
		 * 
		 * @note Provides amortized O(1) insertion and O(1) deletion.
		 * 
		 */
		class group final {
		public:

			auto begin() const noexcept {
				using std::begin;
				return begin(m_packed);
			}

			auto end() const noexcept {
				using std::begin;
				return begin(m_packed) + m_size;
			}

			bool contains(u32 v) const {
				using std::size;
				return v < size(m_sparse)
					&& m_sparse[v] < m_size
					&& m_packed[m_sparse[v]] = v;
			}

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