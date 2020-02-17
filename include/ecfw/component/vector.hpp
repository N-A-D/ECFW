#pragma once

#include <vector>  // std::vector
#include <memory>  // std::unique_ptr
#include <cassert> // assert
#include <ecfw/fwd/vector.hpp>

namespace ecfw {
	namespace detail {

		template <typename T>
		class vector final {
		public:

			using value_type	= T;
			using size_type	    = std::size_t;
			using pointer		= value_type*;
			using const_pointer = const value_type*;

			/**
			 * @brief Access a specific element.
			 * 
			 * @param pos The index of an element.
			 * @return A reference to a specific element.
			 */
			pointer at(size_type pos) {
				using std::as_const;
				return const_cast<pointer>
						(as_const(*this).at(pos));
			}

			/**
			 * @brief Access a specific element.
			 * 
			 * @param pos The index of an element.
			 * @return A const reference to a specific element.
			 */
			const_pointer at(size_type pos) const {
				return static_cast<const T*>(data(pos));
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
			pointer construct(size_type pos, Args&&... args) {
				if (pos >= size())
					accommodate(pos + 1);
				T* comp = static_cast<T*>(data(pos));
				::new (comp) T(std::forward<Args>(args)...);
				return comp;
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
				static_cast<T*>(data(pos))->~T();
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
			 * @return A void pointer to the elements data.
			 */
			void* data(size_type pos) {
				using std::as_const;
				return const_cast<void*>
						(as_const(*this).data(pos));
			}

			/**
			 * @brief Access a specific elements raw data.
			 * 
			 * @param pos The index to a specific element.
			 * @return A const void pointer to the elements data.
			 */
			const void* data(size_type pos) const {
				auto page = m_pages[pos / page_size].get();
				auto offs = (pos & (page_size - 1)) * elem_size;
				return page + offs;
			}

		};

	}
}
