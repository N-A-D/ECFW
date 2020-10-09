#pragma once

#include <tuple>		  // tuple, forward_as_tuple
#include <type_traits>	  // is_const, conditional, conditional_t
#include <boost/hana.hpp> // equal, unique, is_subset
#include <ecfw/detail/buffer.hpp>
#include <ecfw/detail/dword.hpp>
#include <ecfw/detail/sparse_set.hpp>
#include <ecfw/detail/type_list.hpp>

namespace ecfw
{
namespace detail 
{

	/**
	 * @brief Determines the index to an element in an iterable.
	 *
	 * @note
	 * Taken from:
	 * https://stackoverflow.com/questions/33979592/boost-hana-get-index-of-first-matching
	 *
	 * @tparam T Element type.
	 * @tparam Iterable Iterable sequence type.
	 * @param element An element to search for.
	 * @param iterable An iterable sequence to search in.
	 * @return An index to the first occurrence of the element. Otherwise,
	 * the length of the iterable sequence.
	 */
	template <typename T, typename Iterable>
	constexpr auto index_of(const T& element, const Iterable& iterable) {
		auto size = decltype(boost::hana::size(iterable)){};
		auto dropped = decltype(boost::hana::size(
			boost::hana::drop_while(iterable, boost::hana::not_equal.to(element))
		)){};
		return size - dropped;
	}

} // namespace detail

	namespace dtl = detail;
	namespace bh = boost::hana;

	/**
	 * @brief Non-owning collection of entities which share a common
	 * set of components.
	 * 
	 * @tparam Ts Component types common among all entities viewed by *this.
	 */
	template <typename... Ts>
	class view final {
		static constexpr auto viewed = dtl::type_list_v<Ts...>;
		static_assert(bh::equal(bh::unique(viewed), viewed));		
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

		/**
		 * @brief Default copy constructor.
		 * 
		 */
		view(const view&) = default;
		
		/**
		 * @brief Default move constructor.
		 * 
		 */
		view(view&&) = default;

		/**
		 * @brief Default copy assignment operator.
		 * 
		 * @return *this
		 */
		view& operator=(const view&) = default;

		/**
		 * @brief Default move assignment operator.
		 * 
		 * @return *this
		 */
		view& operator=(view&&) = default;

		/**
		 * @brief Returns an iterator to the first element of the view.
		 * 
		 * @return Iterator to the first element.
		 */
		[[nodiscard]] const_iterator begin() const noexcept {
			return m_entities->begin();
		}

		/**
		 * @brief Returns an iterator to the element following the 
		 * last element of the view.
		 * 
		 * @return Iterator to the element following the last element.
		 */
		[[nodiscard]] const_iterator end() const noexcept {
			return m_entities->end();
		}

		/**
		 * @brief Returns a reverse iterator to the first element 
		 * of the reversed view.
		 * 
		 * @return Reverse iterator to the first element.
		 */
		[[nodiscard]] const_reverse_iterator rbegin() const noexcept {
			return m_entities->rbegin();
		}

		/**
		 * @brief Returns a reverse iterataor to the element following
		 * the last element of the reversed view.
		 * 
		 * @return Reverse iterator to the element following the last 
		 *  element.
		 */
		[[nodiscard]] const_reverse_iterator rend() const noexcept {
			return m_entities->rend();
		}

		/**
		 * @brief Checks if there is an entity with identifier equivalent 
		 * to eid in the container.
		 * 
		 * @param eid The entity to look for.
		 * @return true If there is such an entity.
		 * @return false If there does not exist such an entity.
		 */
		[[nodiscard]] bool contains(value_type eid) const {
			return m_entities->contains(eid);
		}

		/**
		 * @brief Returns the number of entities viewed.
		 * 
		 * @return The number of entities viewed.
		 */
		[[nodiscard]] size_type size() const noexcept {
			return m_entities->size();
		}

		/**
		 * @brief Checks if the view has no entities, 
		 * i.e., whether begin() == end().
		 * 
		 * @return true If the view is empty.
		 * @return false If the view is not empty.
		 */
		[[nodiscard]] bool empty() const noexcept {
			return m_entities->empty();
		}

		/**
		 * @brief Returns an entity's components.
		 * 
		 * @note All components are returned when no specific 
		 * components are specified.
		 * 
		 * @tparam Cs The component types to retrieve.
		 * @param eid The entity to fetch for.
		 * @return Reference to a component or a tuple of references.
		 */
		template <typename... Cs>
		[[nodiscard]] decltype(auto) get(uint64_t eid) const {
			assert(contains(eid));
			auto idx = dtl::lsw(eid);
			return unchecked_get<Cs...>(idx);
		}

	private:

		template <typename... Cs>
		[[nodiscard]] decltype(auto) unchecked_get(uint32_t idx) const {
			using std::forward_as_tuple;
			using boost::hana::unique;
			using boost::hana::equal;
			using boost::hana::is_subset;
			
			constexpr auto requested = dtl::type_list_v<Cs...>;
			static_assert(equal(unique(requested), requested));

			static_assert(is_subset(requested, viewed));

			if constexpr (sizeof...(Cs) == 1) {
				auto& buffer = 
					(m_buffers[dtl::index_of(dtl::type_v<Cs>, viewed)], ...);
				return buffer->at(idx);
			}
			else if constexpr (sizeof...(Cs) > 1)
				return forward_as_tuple(get<Cs>(idx)...);
			else 
				return forward_as_tuple(get<Ts>(idx)...);
		}

		view(const dtl::sparse_set& entities, dtl::buffer_type<Ts>&... buffers)
			: m_entities(std::addressof(entities))
			, m_buffers(std::addressof(buffers)...)
		{}

		const dtl::sparse_set* m_entities{};
		boost::hana::tuple<dtl::buffer_type<Ts>*...> m_buffers{};

	};

	/**
	 * @brief Non-owning collection of entities which share a common component.
	 * 
	 * @tparam T Component type common among all entities viewed by *this.
	 */
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

		/**
		 * @brief Default copy constructor.
		 * 
		 */
		view(const view&) = default;
		
		/**
		 * @brief Default move constructor.
		 * 
		 */
		view(view&&) = default;

		/**
		 * @brief Default copy assignment operator.
		 * 
		 * @return *this
		 */
		view& operator=(const view&) = default;

		/**
		 * @brief Default move assignment operator.
		 * 
		 * @return *this
		 */
		view& operator=(view&&) = default;

		/**
		 * @brief Returns an iterator to the first element of the vector.
		 * 
		 * @return Iterator to the first element.
		 */
		[[nodiscard]] const_iterator begin() const noexcept {
			return m_entities->begin();
		}

		/**
		 * @brief Returns an iterator to the element following the last
		 * element of the view.
		 * 
		 * @return Iterator to the element following the last element.
		 */
		[[nodiscard]] const_iterator end() const noexcept {
			return m_entities->end();
		}

		/**
		 * @brief Returns a reverse iterator to the first element of 
		 * the reversed view.
		 * 
		 * @return Reverse iterator to the first element.
		 */
		[[nodiscard]] const_reverse_iterator rbegin() const noexcept {
			return m_entities->rbegin();
		}

		/**
		 * @brief Returns a reverse iterataor to the element following 
		 * the last element of the reversed view.
		 * 
		 * @return Reverse iterator to the element following the last
		 * element.
		 */
		[[nodiscard]] const_reverse_iterator rend() const noexcept {
			return m_entities->rend();
		}

		/**
		 * @brief Checks if there is an entity with identifier equivalent 
		 * to eid in the container.
		 * 
		 * @param eid The entity to look for.
		 * @return true If there is such an entity.
		 * @return false If there does not exist such an entity.
		 */
		[[nodiscard]] bool contains(value_type eid) const {
			return m_entities->contains(eid);
		}

		/**
		 * @brief Returns the number of entities viewed.
		 * 
		 * @return The number of entities viewed.
		 */
		[[nodiscard]] size_type size() const noexcept {
			return m_entities->size();
		}

		/**
		 * @brief Checks if the view has no entities, i.e., size() == 0.
		 * 
		 * @return true If the view is empty.
		 * @return false If the view is not empty.
		 */
		[[nodiscard]] bool empty() const noexcept {
			return m_entities->empty();
		}

		/**
		 * @brief Returns an entity's component.
		 * 
		 * @param eid The entity to fetch for.
		 * @return Reference to the component. 
		 */
		[[nodiscard]] T& get(uint64_t eid) const {
			assert(contains(eid));
			auto idx = dtl::lsw(eid);
			return m_buffer->at(idx);
		}

	private:

		view(const dtl::sparse_set& es, dtl::buffer_type<T>& b)
			: m_entities(std::addressof(es))
			, m_buffer(std::addressof(b))
		{}

		const dtl::sparse_set* m_entities{};
		dtl::buffer_type<T>* m_buffer{};

	};

} // namespace ecfw
