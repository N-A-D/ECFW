#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <iterator>
#include <type_traits>
#include <ecfw/fwd/vector.hpp>

namespace ecfw {
	namespace detail {

		/**
		 * @brief Semi contiguous storage container.
		 * 
		 * The purpose of this class is to serve as the default storage
		 * implementation for components.
		 * 
		 * @note Provides O(1) lookup. Amortized O(1) insert + delete.
		 * 
		 * @tparam T 
		 */
		template <typename T>
		class vector final {
		public:

			using value_type	  = T;
			using size_type		  = std::size_t;
			using reference		  = value_type&;
			using const_reference = const value_type&;

			/**
			 * @brief Access a specific element.
			 * 
			 * @param pos The index of an element.
			 * @return A reference to a specific element.
			 */
			reference at(size_type pos) {
				using std::as_const;
				return const_cast<reference>
						(as_const(*this).at(pos));
			}

			/**
			 * @brief Access a specific element.
			 * 
			 * @param pos The index of an element.
			 * @return A const reference to a specific element.
			 */
			const_reference at(size_type pos) const {
				return *reinterpret_cast<T*>(data(pos));
			}

			/**
			 * @brief Inserts a new element in the vector.
			 * 
			 * @note Inserting onto an existing element is undefined.
			 * 
			 * @pre The index to insert into must point to an uninitialized
			 * element.
			 * 
			 * @tparam Args Component constructor argument types.
			 * @param pos The index to a specific element.
			 * @param args Component constructor argument values.
			 * @return A reference to the newly constructed element.
			 */
			template <typename... Args>
			reference construct(size_type pos, Args&&... args) {
				if (pos >= size())
					accommodate(pos + 1);
				T* comp = reinterpret_cast<T*>(data(pos));
				::new (comp) T{std::forward<Args>(args)...};
				return *comp;
			}

			/**
			 * @brief Deletes an element.
			 * 
			 * @note Deleting an uninitialized element is undefined.
			 * 
			 * @pre The specified element must be initialized.
			 * 
			 * @param pos The index to a specific element.
			 */
			void destroy(size_type pos) {
				reinterpret_cast<T*>(data(pos))->~T();
			}

			/**
			 * @brief Returns the number of element in the container.
			 * 
			 * @note The size is not indicative of the number of initialized
			 * elements.
			 * 
			 * @return The number of elements.
			 */
			size_type size() const noexcept {
				return std::size(m_pages) * page_size;
			}

		private:

			using byte = unsigned char;

			static constexpr size_type elem_size = sizeof(T);
			static constexpr size_type page_size = 4096;

			std::vector<std::unique_ptr<byte[]>> m_pages{};

			/**
			 * @brief Ensures that a specific index fits within bounds.
			 * 
			 * @param pos The index to accommodate.
			 */
			void accommodate(size_type pos) {
				using std::make_unique;
				auto sz = page_size * elem_size;
				while (size() < pos)
					m_pages.push_back(make_unique<byte[]>(sz));
			}

			/**
			 * @brief Access a specific elements raw data.
			 * 
			 * @param pos The index to a specific element.
			 * @return A byte pointer to the elements data.
			 */
			byte* data(size_type pos) {
				using std::as_const;
				return const_cast<byte*>
						(as_const(*this).data(pos));
			}

			/**
			 * @brief Access a specific elements raw data.
			 * 
			 * @param pos The index to a specific element.
			 * @return A const byte pointer to the elements data.
			 */
			const byte* data(size_type pos) const {
				auto page = m_pages[pos / page_size].get();
				auto offs = (pos & (page_size - 1)) * elem_size;
				return page + offs;
			}

		};

	}
}
