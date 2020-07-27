#pragma once

#include <array>					  // array
#include <type_traits>				  // is_const, conditional, conditional_t
#include <ecfw/detail/buffer.hpp>
#include <ecfw/detail/integers.hpp>
#include <ecfw/detail/sparse_set.hpp>

namespace ecfw
{

	namespace dtl = detail;

	template <typename... Ts>
	class view final {
	public:
		friend class world;

		using difference_type = dtl::sparse_set::difference_type;
		using size_type = dtl::sparse_set::size_type;
		using value_type = dtl::sparse_set::value_type;
		using const_reference = dtl::sparse_set::const_reference;
		using reference = dtl::sparse_set::reference;
		using const_pointer = dtl::sparse_set::const_pointer;
		using pointer = dtl::sparse_set::pointer;
		using const_iterator = dtl::sparse_set::const_iterator;
		using iterator = dtl::sparse_set::iterator;
		using const_reverse_iterator = dtl::sparse_set::const_reverse_iterator;
		using reverse_iterator = dtl::sparse_set::reverse_iterator;

		const_iterator begin() const noexcept {
			return m_entities->begin();
		}

		const_iterator end() const noexcept {
			return m_entities->end();
		}

		const_iterator rbegin() const noexcept {
			return m_entities->rbegin();
		}

		const_iterator rend() const noexcept {
			return m_entities->rend();
		}

		bool contains(value_type eid) const {
			return m_entities->contains(eid);
		}

		size_type size() const noexcept {
			return m_entities->size();
		}

		bool empty() const noexcept {
			return m_entities->empty();
		}

	private:

		const dtl::sparse_set* m_entities{};
	};

	template <typename T>
	class view<T> final {
	public:
		friend class world;
		
		using difference_type		 = dtl::sparse_set::difference_type;
		using size_type				 = dtl::sparse_set::size_type;
		using value_type			 = dtl::sparse_set::value_type;
		using const_reference		 = dtl::sparse_set::const_reference;
		using reference				 = dtl::sparse_set::reference;
		using const_pointer			 = dtl::sparse_set::const_pointer;
		using pointer				 = dtl::sparse_set::pointer;
		using const_iterator		 = dtl::sparse_set::const_iterator;
		using iterator				 = dtl::sparse_set::iterator;
		using const_reverse_iterator = dtl::sparse_set::const_reverse_iterator;
		using reverse_iterator		 = dtl::sparse_set::reverse_iterator;

		const_iterator begin() const noexcept {
			return m_entities->begin();
		}

		const_iterator end() const noexcept {
			return m_entities->end();
		}

		const_iterator rbegin() const noexcept {
			return m_entities->rbegin();
		}

		const_iterator rend() const noexcept {
			return m_entities->rend();
		}

		bool contains(value_type eid) const {
			return m_entities->contains(eid);
		}

		size_type size() const noexcept {
			return m_entities->size();
		}

		bool empty() const noexcept {
			return m_entities->empty();
		}

	private:

		const dtl::sparse_set* m_entities{};

	};

}
