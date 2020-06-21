#pragma once

#include <ecfw/aliases/bitset2.hpp>
#include <ecfw/aliases/hana.hpp>

namespace ecfw { namespace detail {

	template <
		std::size_t N
	> using bitmask = bs2::bitset2<N>;

	/**
	 * @brief Determines the index to an element in an iterable.
	 *
	 * @note 
	 * Taken from: 
	 * https://stackoverflow.com/questions/33979592/boost-hana-get-index-of-first-matching
	 *
	 * @tparam T: The element type.
	 * @tparam Iterable: The iterable sequence type.
	 * @param element: An element to search for.
	 * @param iterable: An iterable sequence to search in.
	 * @return An index to the first occurence of the element. Otherwise,
	 * the length of the iterable sequence.
	 */
	template <
		typename T, 
		typename Iterable
	> constexpr auto index_of(const T& element, const Iterable& iterable) {
		auto size = decltype(hana::size(iterable)){};
		auto dropped = decltype(hana::size(
			hana::drop_while(iterable, hana::not_equal.to(element))
		)){};
		return size - dropped;
	}
	
	/**
	 * @brief Encodes a subsequence as a bitmask.
	 * 
	 * The purpose of this function is to provide compile-time 
	 * bitmasks that can be used to index entities based on the
	 * components they have.
	 * 
	 * @tparam Xs: An iterable sequence type.
	 * @tparam Ys: An iterable sequence type.
	 * @param xs: A sequence of types that is a subset of ys.
	 * @param ys: A sequence of types that is a superset set of xs.
	 * @return A bitmask whose active bits represent elements of the subset.
	 */
	template <
		typename Xs, 
		typename Ys
	> constexpr auto encode_subset_as_bitmask(const Xs& xs, const Ys& ys) {
		static_assert(hana::is_subset(decltype(xs){}, decltype(ys){}));
		auto size = decltype(hana::size(ys)){};
		bitmask<size> mask;
		auto encode = [&](auto t) { 
			mask.set(index_of(t, ys)); 
		};
		hana::for_each(xs, encode);
		return mask;
	}
	
} }