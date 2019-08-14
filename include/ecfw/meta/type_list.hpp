#pragma once

#include <type_traits>

namespace ecfw {

	template <class...>
	struct type_list;

	namespace meta {

		template <
			class T, 
			class TypeList
		> struct contains;

		template <
			class T,
			class... Ts
		> struct contains<T, type_list<Ts...>>
			: std::disjunction<std::is_same<T, Ts>...> {};

		template <
			class T,
			class TypeList
		> constexpr bool contains_v = contains<T, TypeList>::value;

		template <
			class TypeList
		> struct is_unique;

		template <>
		struct is_unique<type_list<>>
			: std::true_type {};

		template <
			class T,
			class... Ts
		> struct is_unique<type_list<T, Ts...>>
			: std::conjunction
				<
					std::negation<contains<T, type_list<Ts...>>>, 
					is_unique<type_list<Ts...>>
				> {};

		template <
			class TypeList
		> constexpr bool is_unique_v = is_unique<TypeList>::value;

		template <
			class T,
			class TypeList
		> struct index_of;

		template <
			class T,
			class... Ts
		> struct index_of<T, type_list<T, Ts...>>
			: std::integral_constant<size_t, 0> {};

		template <
			class T,
			class U,
			class... Us
		> struct index_of<T, type_list<U, Us...>>
			: std::integral_constant
				<size_t, 1 + index_of<T, type_list<Us...>>::value> {};

		template <
			class T,
			class TypeList
		> constexpr size_t index_of_v = index_of<T, TypeList>::value;

		template <
			class Subset,
			class Set
		> struct is_subset;

		template <
			class... Ts,
			class... Us
		> struct is_subset<type_list<Ts...>, type_list<Us...>>
			: std::conjunction<contains<Ts, type_list<Us...>>...> {};

		template <
			class Subset,
			class Set
		> constexpr bool is_subset_v = is_subset<Subset, Set>::value;

	}
}