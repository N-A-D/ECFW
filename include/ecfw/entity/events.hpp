#pragma once

namespace ecfw {

	template <
		class Entity
	> struct entity_created
	{
		Entity entity;
	};

	template <
		class Entity
	> struct entity_destroyed
	{
		Entity entity;
	};

	template <
		class Entity,
		class Component
	> struct component_added
	{
		Entity entity;
		Component& component;
	};

	template <
		class Entity,
		class Component
	> struct component_removed
	{
		Entity entity;
		Component& component;
	};

}