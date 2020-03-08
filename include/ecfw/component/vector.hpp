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
	class paged_vector final {
	public:

		using value_type	= T;
		using size_type		= std::size_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;

		/**
		 * @brief Default constructor.
		 * 
		 */
		paged_vector() = default;

		/**
		 * @brief Default move constructor
		 * 
		 */
		paged_vector(paged_vector&&) = default;

		/**
		 * @brief Default move assignment operator.
		 * 
		 * @return *this.
		 */
		paged_vector& operator=(paged_vector&&) = default;

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
			if (pos >= size())
				accommodate(pos + 1);
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
		 * @brief Ensures that a specific index fits within bounds.
		 *
		 * @param pos The index to accommodate.
		 */
		void accommodate(size_type pos) {
			using std::make_unique;
			while (size() < pos)
				m_pages.push_back(make_unique<byte[]>(page_size_bytes));
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
			auto offs = (pos % page_size) * elem_size;
			return page + offs;
		}

	};

}
