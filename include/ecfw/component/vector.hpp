#pragma once

#include <vector>  // vector
#include <memory>  // unique_ptr
#include <cassert> // assert

namespace ecfw {

	/**
	 * @brief A dynamically resizing container which allocates
	 * memory in pages.
	 *
	 * The purpose of this container is to serve as the default
	 * storage implementation for components.
	 * 
	 * @note Leaves uninitialized gaps for elements not in use.
	 * @note Its the responsibility of users who construct elements
	 * to correspondingly destroy them using the appropriate interface
	 * functions for each.
	 *  
	 * @tparam T The type of element to store.
	 * @tparam PageSize The number of elements stored in each page.
	 */
	template <typename T, std::size_t PageSize = 4096>
	class page_vector final {
	public:

		using value_type	= T;
		using size_type		= std::size_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;

		/**
		 * @brief Default constructor.
		 * 
		 */
		page_vector() = default;

		/**
		 * @brief Default move constructor
		 * 
		 */
		page_vector(page_vector&&) = default;

		/**
		 * @brief Default move assignment operator.
		 * 
		 * @return *this.
		 */
		page_vector& operator=(page_vector&&) = default;

		/**
		 * @brief Access a specific element.
		 *
		 * @param pos The index of an element.
		 * @return A pointer to a specific element.
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
		 * @return A const pointer to a specific element.
		 */
		const_pointer at(size_type pos) const {
			assert(m_pages[page(pos)]);
			return static_cast<const_pointer>(data(pos));
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
		 * @return A pointer to the newly constructed element.
		 */
		template <typename... Args>
		pointer construct(size_type pos, Args&&... args) {
			ensure_memory_at(pos);
			pointer comp = static_cast<pointer>(data(pos));
			::new (comp) value_type(std::forward<Args>(args)...);
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
			assert(m_pages[page(pos)]);
			static_cast<pointer>(data(pos))->~T();
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
		static constexpr size_type page_size = PageSize;
		static constexpr size_type page_size_bytes = elem_size * page_size;

		std::vector<std::unique_ptr<byte[]>> m_pages{};

		/**
		 * @brief Ensures the memory at the given index.
		 *
		 * @param pos The index to accommodate.
		 */
		void ensure_memory_at(size_type pos) {
			using std::make_unique;
			size_type page_n = page(pos);
			if (pos >= size())
				m_pages.resize(page_n + 1);
			if (!m_pages[page_n])
				m_pages[page_n] = 
					make_unique<byte[]>(page_size_bytes);
		}

		/**
		 * @brief Returns an index to a memory page.
		 *
		 * @param pos An index to map to a memory page.
		 */
		size_type page(size_type pos) const noexcept {
			return pos / page_size;
		}

		/**
		 * @brief Returns a memory page offset.
		 *
		 * @param pos The index to offset.
		 */
		size_type offset(size_type pos) const noexcept {
			if constexpr ((page_size & page_size - 1) == 0)
				return (pos & page_size - 1) * elem_size;
			else
				return (pos % page_size) * elem_size;
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
			return m_pages[page(pos)].get() + offset(pos);
		}

	};

}
