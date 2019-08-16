#pragma once

#include <stack>
#include <execution>
#include <algorithm>
#include <functional>
#include <ecfw/entity/group.hpp>
#include <ecfw/entity/events.hpp>
#include <ecfw/component/mask.hpp>
#include <ecfw/component/storage.hpp>
#include <ecfw/meta/type_list.hpp>
#include <ecfw/meta/type_traits.hpp>
#include <ecfw/event/dispatcher.hpp>
#include <ancillary/container/flat_map.hpp>

namespace ecfw {

	template <
		class Entity,
		class CompList
	> class entity_manager;

	template <
		class Entity,
		class... Ts
	> class entity_manager<Entity, type_list<Ts...>> {

		static_assert(meta::is_unique_v<type_list<Ts...>>, "Duplicate component type(s)!");
		static_assert(std::conjunction_v
			<
				std::disjunction
					<
						std::is_move_constructible<Ts>, 
						std::is_copy_constructible<Ts>
					>...
			>, "Each component must be either copy or  move constructible!");

		using comp_list = type_list<Ts...>;
		using comp_mask = detail::comp_mask<Ts...>;

	public:

		using entity_type = Entity;
		using traits_type = entity_traits<entity_type>;
		using group_type  = entity_group<entity_type>;

		using event_list = type_list
							<
								entity_created<entity_type>, 
								entity_destroyed<entity_type>, 
								component_added<entity_type, Ts>..., 
								component_removed<entity_type, Ts>...
							>;

		using event_dispatcher_type = event_dispatcher<event_list>;

	private:

		group_type m_recycle_list;
		std::vector<entity_type> m_entities;
		std::vector<comp_mask> m_comp_masks;
		std::stack<size_t, std::vector<size_t>> m_free_list;
		std::tuple<underlying_storage_t<Ts>...> m_comp_pools;
		ancillary::flat_map<comp_mask, group_type> m_groups;
		event_dispatcher_type m_event_dispatcher;

		// INTERNAL IMPLEMENTATION 

		void enable_component(size_t cpos, entity_type e) {
			m_comp_masks[traits_type::index(e)].set(cpos);
		}

		void disable_component(size_t cpos, entity_type e) {
			m_comp_masks[traits_type::index(e)].reset(cpos);
		}

		bool check_component(size_t cpos, entity_type e) const {
			return m_comp_masks[traits_type::index(e)].test(cpos);
		}

		template <
			class C
		> void try_remove_component(bool test, 
			typename traits_type::index_type idx) 
		{
			if (test)
				std::get<underlying_storage_t<C>>(m_comp_pools).destroy(idx);
		}

		void remove_components(entity_type e) {
			auto idx = traits_type::index(e);
			const comp_mask& mask = m_comp_masks[idx];
			(try_remove_component<Ts>(mask.test(meta::index_of_v<Ts, comp_list>), idx), ...);
		}

		const group_type& group_entities_matching(const comp_mask& id) {
			auto it = m_groups.find(id);
			if (it != m_groups.end()) // cache found return it
				return it->second;
			else { // build the cache
				group_type group;
				for (size_t i = 0; i != m_entities.size(); ++i) {
					const comp_mask& mask = m_comp_masks[i];
					if ((mask & id) == id) // entity owns the components
						group.emplace_hint(group.end(), m_entities[i]);
				}
				auto ret = m_groups.emplace_hint(it, id, group);
				return ret->second;
			}
		}

		void group_entity_using(size_t cpos, entity_type e) {
			const comp_mask& mask = m_comp_masks[traits_type::index(e)];
			for (auto&[id, group] : m_groups)
				if (id.test(cpos) && (mask & id) == id)
					group.emplace(e);
		}
		
		void degroup_entity_using(size_t cpos, entity_type e) {
			for (auto&[id, group] : m_groups)
				if (id.test(cpos))
					group.erase(e);
		}
		
		void degroup_entity(entity_type e) {
			for (auto&[_, group] : m_groups)
				group.erase(e);
		}

		void recycle_entity(entity_type e) {
			degroup_entity(e);
			remove_components(e);
			auto idx = traits_type::index(e);
			m_entities[idx] = traits_type::recycle(e);
			m_comp_masks[idx].reset();
			m_free_list.push(idx);
		}

	public:

		entity_manager() = default;

		entity_manager(size_t count)
			: entity_manager()
		{
			m_entities.reserve(count);
			m_comp_masks.reserve(count);
			reserve<Ts...>(count);
		}

		template <
			class... Cs
		> entity_type create() {
			static_assert(meta::is_unique_v<type_list<Cs...>>, 
				"Duplicate component type(s)!");
			static_assert(meta::is_subset_v<type_list<Cs...>, comp_list>, 
				"Unknown component type(s)!");
			static_assert(std::conjunction_v<std::is_default_constructible<Cs>...>,
				"Non-default constructible component type(s)!");
			entity_type e;
			if (m_free_list.empty()) {
				using idx_t = typename traits_type::index_type;
				assert(traits_type::max_index >= static_cast<idx_t>(m_entities.size()));
				e = traits_type::create(static_cast<idx_t>(m_entities.size()), 0);
				m_entities.emplace_back(e);
				m_comp_masks.emplace_back(0);
			}
			else {
				e = m_entities[m_free_list.top()];
				m_free_list.pop();
			}
			if constexpr (sizeof...(Cs) > 0)
				(assign<Cs>(e), ...);
			m_event_dispatcher(entity_created<entity_type>{ e });
			return e;
		}

		template <
			class... Cs
		> void create(size_t n) {
			for (size_t i = 0; i != n; ++i)
				create<Cs...>();
		}

		template <
			class... Cs, 
			class InIt, 
			class = std::enable_if_t<meta::is_iterator_v<InIt>>
		> void create(InIt first, InIt last) {
			static_assert(std::is_convertible_v
				<
					typename std::iterator_traits<InIt>::value_type,
					entity_type
				>, "Incompatible iterator value_type!");
			std::generate(first, last, [this]() mutable {
				return create<Cs...>();
			});
		}

		template <
			class C, 
			class... Cs
		> entity_type clone(entity_type model) {
			static_assert(meta::is_unique_v<type_list<C, Cs...>>,
				"Duplicate component type(s)!");
			static_assert(meta::is_subset_v<type_list<C, Cs...>, comp_list>,
				"Unknown component type(s)!");
			static_assert(std::conjunction_v<std::is_copy_constructible<C>, 
				std::is_copy_constructible<Cs>...>, 
				"Non-copy constructible component type(s)!");
			entity_type e = create();
			assign<C>(e, component<C>(model));
			(assign<Cs>(e, component<Cs>(model)), ...);
			return e;
		}

		template <
			class C, 
			class... Cs
		> void clone(entity_type model, size_t n) {
			for (size_t i = 0; i != n; ++i)
				clone<C, Cs...>(model);
		}

		template <
			class C, 
			class... Cs, 
			class InIt, 
			class = std::enable_if_t<meta::is_iterator_v<InIt>>
		> void clone(entity_type model, InIt first, InIt last) {
			static_assert(std::is_convertible_v
				<
					typename std::iterator_traits<InIt>::value_type,
					entity_type
				>, "Incompatible iterator value_type");
			std::generate(first, last, [this, model]() mutable { 
				return clone<C, Cs...>(model); 
			});
		}

		void destroy(entity_type e) {
			assert(valid(e));
			m_recycle_list.emplace(e);
			assert(valid(e));
		}

		template <
			class InIt, 
			class = std::enable_if_t<meta::is_iterator_v<InIt>>
		> void destroy(InIt first, InIt last) {
			static_assert(std::is_convertible_v
				<
					typename std::iterator_traits<InIt>::value_type,
					entity_type
				>, "Incompatible iterator value_type!");
			for (; first != last; ++first)
				destroy(*first);
		}
		
		template <
			class C
		> bool empty() const noexcept {
			return std::get<underlying_storage_t<C>>(m_comp_pools).empty();
		}

		template <
			class C
		> size_t size() const noexcept {
			return std::get<underlying_storage_t<C>>(m_comp_pools).size();
		}

		template <
			class... Cs
		> void reserve(size_t cap) {
			(std::get<underlying_storage_t<Cs>>(m_comp_pools).reserve(cap), ...);
		}

		template <
			class... Cs
		> bool has(entity_type e) const {
			static_assert(meta::is_subset_v<type_list<Cs...>, comp_list>
				, "Unknown component type(s)!");
			assert(valid(e));
			return (check_component(meta::index_of_v<Cs, comp_list>, e) && ...);
		}

		template <
			class C,
			class... Args
		> C& assign(entity_type e, Args&&... args) {
			assert(!has<C>(e));
			size_t cpos = meta::index_of_v<C, comp_list>;
			enable_component(cpos, e);
			group_entity_using(cpos, e);
			auto idx = traits_type::index(e);
			C& comp = std::get<underlying_storage_t<C>>(m_comp_pools)
						.construct(idx, std::forward<Args>(args)...);
			m_event_dispatcher(component_added<entity_type, C>{ e, comp });
			assert(has<C>(e));
			return comp;
		}

		template <
			class C
		> C& component(entity_type e) {
			return const_cast<C&>(
				const_cast<const entity_manager*>(this)->component<C>(e));
		}

		template <
			class C
		> const C& component(entity_type e) const {
			assert(has<C>(e));
			return std::get<underlying_storage_t<C>>(m_comp_pools).get(traits_type::index(e));
		}

		template <
			class... Cs
		> void remove(entity_type e) {
			assert(has<Cs...>(e));
			auto idx = traits_type::index(e);
			(disable_component(meta::index_of_v<Cs, comp_list>, e), ...);
			(m_event_dispatcher(
				component_removed<entity_type, Cs>{ 
					e, std::get<underlying_storage_t<Cs>>(m_comp_pools).get(idx) 
				}),
			...);
			(std::get<underlying_storage_t<Cs>>(m_comp_pools).destroy(idx), ...);
			(degroup_entity_using(meta::index_of_v<Cs, comp_list>, e), ...);
			assert(!has<Cs...>(e));
		}

		template <
			class... Cs,
			class Transform
		> void entities_with(Transform func) {
			static_assert(meta::is_unique_v<type_list<Cs...>>);
			static_assert(meta::is_subset_v<type_list<Cs...>, comp_list>);
			static_assert(std::disjunction_v<
				std::is_invocable<Transform, Cs&...>,
				std::is_invocable<Transform, entity_type, Cs&...>
			>);
			
			comp_mask group_id;
			(group_id.set(meta::index_of_v<Cs, comp_list>), ...);

			const group_type& group = group_entities_matching(group_id);
			for (auto e : group) {
				auto idx = traits_type::index(e);
				if constexpr (std::is_invocable_v<Transform, Cs&...>)
					func(std::get<underlying_storage_t<Cs>>(m_comp_pools).get(idx)...);
				else
					func(e, std::get<underlying_storage_t<Cs>>(m_comp_pools).get(idx)...);
			}
		}

		template <
			class... Cs,
			class ExecPolicy,
			class Transform
		> void entities_with(ExecPolicy&& policy, Transform func) {
			static_assert(meta::is_unique_v<type_list<Cs...>>);
			static_assert(meta::is_subset_v<type_list<Cs...>, comp_list>);
			static_assert(std::is_invocable_v<Transform, Cs&...>);

			comp_mask group_id;
			(group_id.set(meta::index_of_v<Cs, comp_list>), ...);

			const group_type& group = group_entities_matching(group_id);
			std::for_each(std::forward<ExecPolicy>(policy),
				group.begin(), group.end(), [this, &func](auto e) {
				auto idx = traits_type::index(e);
				func(std::get<underlying_storage_t<Cs>>(m_comp_pools).get(idx)...);
			});
		}

		template <
			class... Cs
		> size_t num_entities_with() {
			static_assert(meta::is_unique_v<type_list<Cs...>>);
			static_assert(meta::is_subset_v<type_list<Cs...>, comp_list>);
			return std::count_if(m_entities.begin(), m_entities.end(), [this](auto e) {
				return this->has<Cs...>(e);
			});
		}

		event_dispatcher_type& events() {
			return m_event_dispatcher;
		}

		bool valid(entity_type e) const {
			auto idx = traits_type::index(e);
			return m_entities[idx] == e || m_recycle_list.contains(e);
		}

		size_t num_entities() const {
			return m_entities.size();
		}

		size_t num_live_entities() const {
			return num_entities() - num_reusable_entities();
		}

		size_t num_recyclable_entities() const {
			return m_recycle_list.size();
		}

		size_t num_reusable_entities() const {
			return m_free_list.size();
		}

		void update() {
			auto it = m_recycle_list.end();
			while (it > m_recycle_list.begin()) {
				--it;
				auto e = *it;
				recycle_entity(e);
				it = m_recycle_list.erase(it);
				m_event_dispatcher(entity_destroyed<entity_type>{ e });
			}
		}

		void reset() {
			(std::get<underlying_storage_t<Ts>>(m_comp_pools).clear(), ...);
			while (!m_free_list.empty())
				m_free_list.pop();
			m_groups.clear();
			m_comp_masks.clear();
			m_entities.clear();
			m_recycle_list.clear();
		}
	};
}