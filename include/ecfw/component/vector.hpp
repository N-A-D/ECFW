#pragma once

#include <vector>  // vector
#include <memory>  // unique_ptr
#include <cassert> // assert
#include <ecfw/fwd/vector.hpp>

namespace ecfw {

	/**
	 * @brief A dynamically resizing container which allocates
	 * memory in fixed sized blocks.
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
	 * @tparam BlockSize The number of elements stored in each block.
	 */
	template <
		typename T, 
		std::size_t BlockSize
	> class block_vector final {
	public:

		using value_type	= T;
		using size_type		= std::size_t;
		using pointer		= value_type*;
		using const_pointer = const value_type*;

		/**
		 * @brief Default constructor.
		 * 
		 */
		block_vector() = default;

		/**
		 * @brief Default move constructor
		 * 
		 */
		block_vector(block_vector&&) = default;

		/**
		 * @brief Default move assignment operator.
		 * 
		 * @return *this.
		 */
		block_vector& operator=(block_vector&&) = default;

		/**
		 * @brief Access a specific element.
		 *
		 * @param pos: The index to an element.
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
		 * @param pos: The index to an element.
		 * @return A const_pointer to a specific element.
		 */
		const_pointer at(size_type pos) const {
			assert(m_blocks[block(pos)]);
			return static_cast<const_pointer>(data(pos));
		}

		/**
		 * @brief Inserts a new element in the vector.
		 *
		 * @note Inserting onto an existing element has undefined behaviour.
		 *
		 * @pre The index to insert into must point to an uninitialized
		 * element.
		 *
		 * @tparam Args Component constructor argument types.
		 * @param pos: The index to a specific element.
		 * @param args: Component constructor argument values.
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
		 * @note Deleting an uninitialized element has undefined behaviour.
		 *
		 * @pre The specified element must be initialized.
		 *
		 * @param pos: The index to a specific element.
		 */
		void destroy(size_type pos) {
			assert(m_blocks[block(pos)]);
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
			return std::size(m_blocks) * block_size;
		}

	private:

		using byte = unsigned char;

		static constexpr size_type elem_size = sizeof(T);
		static constexpr size_type block_size = BlockSize;
		static constexpr size_type block_size_bytes = elem_size * block_size;

		std::vector<std::unique_ptr<byte[]>> m_blocks{};

		/**
		 * @brief Ensures the memory at  the given index is allocated.
		 *
		 * @param pos: The index to accommodate.
		 */
		void ensure_memory_at(size_type pos) {
			using std::make_unique;
			size_type block_n = block(pos);
			if (pos >= size())
				m_blocks.resize(block_n + 1);
			if (!m_blocks[block_n])
				m_blocks[block_n] = 
					make_unique<byte[]>(block_size_bytes);
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
		 * @param pos: The index to offset into a block.
		 */
		size_type offset(size_type pos) const noexcept {
			if constexpr ((block_size & block_size - 1) == 0)
				return (pos & block_size - 1) * elem_size;
			else
				return (pos % block_size) * elem_size;
		}

		/**
		 * @brief Access a specific elements raw data.
		 *
		 * @param pos: The index to a specific element.
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
		 * @param pos: The index to a specific element.
		 * @return A const void pointer to the elements data.
		 */
		const void* data(size_type pos) const {
			return m_blocks[block(pos)].get() + offset(pos);
		}

	};

}
