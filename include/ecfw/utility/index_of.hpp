#pragma once

#include <ecfw/aliases/hana.hpp>

namespace ecfw {
	namespace detail {

        /**
         * @brief Determines the index to an element in an iterable.
         * 
         * @note Taken from: https://stackoverflow.com/questions/33979592/boost-hana-get-index-of-first-matching
         * 
         * @tparam Iterable A boost::hana::tuple
         * @tparam T The element type.
         * @param iterable A tuple to search.
         * @param element An element to search for.
         * @return An index to the first occurence of the element. Otherwise,
         * the length of the iterable sequence.
         */
        template <typename Iterable, typename T>
        constexpr auto index_of(const Iterable& iterable, const T& element) {
            auto size = decltype(hana::size(iterable)){};
            auto dropped = decltype(hana::size(
                hana::drop_while(iterable, hana::not_equal.to(element))
            )){};
            return size - dropped;
        }

	}
}