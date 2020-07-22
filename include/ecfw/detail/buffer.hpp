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

		base_buffer(size_t object_size, size_t block_size)
			: m_blocks{}
			, m_object_size{object_size}
			, m_block_size{block_size}
		{}

		virtual ~base_buffer() = default;
		
		const void* data(size_t index) const {
			assert(valid(index));
			return m_blocks[block(index)].get() + offset(index);
		}

		void* data(size_t index) {
			using std::as_const;
			return const_cast<void*>(as_const(*this).data(index));
		}

		void accommodate(size_t index) {
			using std::make_unique;
			size_t block_i = block(index);
			if (block_i >= m_blocks.size())
				m_blocks.resize(block_i + 1);
			if (!m_blocks[block_i]) {
				size_t size_in_bytes =
					m_object_size * m_block_size;
				m_blocks[block_i] = 
					make_unique<unsigned char[]>(size_in_bytes);
			}
		}

		size_t size() const noexcept {
			return static_cast<size_t>(m_blocks.size()) * m_block_size;
		}

		bool valid(size_t index) const {
			return block(index) < m_blocks.size()
				&& m_blocks[block(index)];
		}

		virtual void destroy(size_t) = 0;

	private:
		
		size_t block(size_t index) const noexcept {
			return index / m_block_size;
		}

		size_t offset(size_t index) const noexcept {
			return (index % m_block_size) * m_object_size;
		}

		std::vector<std::unique_ptr<unsigned char[]>> m_blocks{};
		size_t m_object_size{};
		size_t m_block_size{};
	};

	template <typename T>
	class typed_buffer final : public base_buffer {
	public:
		explicit typed_buffer(size_t block_size = 8192)
			: base_buffer(sizeof(T), block_size)
		{}

		void destroy(size_t index) override {
			static_cast<T*>(data(index))->~T();
		}
	};

} // namespace detail
} // namespace ecfw