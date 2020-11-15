#pragma once

#include <boost/hana/set.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/minus.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/drop_while.hpp>
#include <boost/hana/integral_constant.hpp>

namespace ecfw {
namespace detail {

    namespace bh = boost::hana;

    template <typename T>
    using type = bh::basic_type<T>;

    template <typename T>
    constexpr auto type_v = type<T>{};

    template <typename... Ts>
    using type_list = bh::tuple<bh::basic_type<Ts>...>;

    template <typename... Ts>
    constexpr auto type_list_v = type_list<Ts...>{};

    template <typename Iterable>
    constexpr auto is_unique(const Iterable& iterable) {
        return bh::size(bh::to<bh::set_tag>(iterable)) == bh::size(iterable);
    }

    template <typename T, typename Iterable>
    constexpr auto index_of(const T& element, const Iterable& iterable) {
        auto size = decltype(bh::size(iterable)){};
        auto dropped = decltype(bh::size(
            bh::drop_while(iterable, bh::not_equal.to(element))
        )){};
        return size - dropped;
    }

} // namespace detail
} // namespace ecfw