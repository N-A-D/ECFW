#pragma once

#include <vector>   // vector
#include <memory>   // unique_ptr
#include <utility>  // as_const
#include <cassert>  // assert

namespace ecfw 
{
namespace detail 
{

	class base_buffer {
	public:

		base_buffer(uint32_t object_size, uint32_t block_size)
			: m_blocks{}
			, m_object_size{object_size}
			, m_block_size{block_size}
		{}

		virtual ~base_buffer() = default;
		
		const void* data(uint32_t index) const {
			assert(valid(index));
			return m_blocks[block(index)].get() + offset(index);
		}

		void* data(uint32_t index) {
			using std::as_const;
			return const_cast<void*>(as_const(*this).data(index));
		}

		void accommodate(uint32_t index) {
			using std::make_unique;

			// Only allocate space for the block which includes
			// the given index. This is to reduce the amount of
			// wasted space in case only a few entities have 
			// data stored in *this.
			uint32_t block_i = block(index);
			if (block_i >= m_blocks.size())
				m_blocks.resize(block_i + 1);
			if (!m_blocks[block_i]) {
				uint32_t size_in_bytes =
					m_object_size * m_block_size;
				m_blocks[block_i] = 
					make_unique<unsigned char[]>(size_in_bytes);
			}
		}

		void reserve(uint32_t n) {
			using std::make_unique;

			uint32_t block_n = block(n);
			if (block_n >= m_blocks.size())
				m_blocks.resize(block_n + 1);

			// Unlike accommodate(...), we need to allocate space
			// for all indices up to and including n. Therefore,
			// each block up to and including the block which 
			// includes index n must be allocated.
			uint32_t size_in_bytes = m_object_size * m_block_size;
			for (uint32_t block_i = 0; block_i != block_n; ++block_i) {
				// Just in case there are existing components
				if (!m_blocks[block_i])
					m_blocks[block_i] = 
						make_unique<unsigned char[]>(size_in_bytes);
			}
		}

		uint32_t size() const noexcept {
			return static_cast<uint32_t>(m_blocks.size()) * m_block_size;
		}

		bool valid(uint32_t index) const {
			return block(index) < m_blocks.size()
				&& m_blocks[block(index)];
		}

		virtual void destroy(uint32_t) = 0;

	private:
		
		uint32_t block(uint32_t index) const noexcept {
			return index / m_block_size;
		}

		uint32_t offset(uint32_t index) const noexcept {
			return (index % m_block_size) * m_object_size;
		}

		std::vector<std::unique_ptr<unsigned char[]>> m_blocks{};
		uint32_t m_object_size{};
		uint32_t m_block_size{};
	};

	template <typename T>
	class typed_buffer final : public base_buffer {
	public:
		explicit typed_buffer(uint32_t block_size = 8192)
			: base_buffer(sizeof(T), block_size)
		{}

		void destroy(uint32_t index) override {
			static_cast<T*>(data(index))->~T();
		}
	};

} // namespace detail
} // namespace ecfw