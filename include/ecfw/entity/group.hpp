#pragma once

#include <ecfw/entity/traits.hpp>
#include <ancillary/container/flat_set.hpp>

namespace ecfw {

	template <class Entity>
	struct entity_compare {
		bool operator()(Entity lhs, Entity rhs) const {
			using traits_type = entity_traits<Entity>;
			return traits_type::index(lhs) < traits_type::index(rhs);
		}
	};

	template <class Entity>
	using entity_group = ancillary::flat_set<Entity, entity_compare<Entity>>;

}