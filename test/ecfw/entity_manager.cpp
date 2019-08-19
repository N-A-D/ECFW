#include <gtest/gtest.h>
#include <ecfw/entity/manager.hpp>

template <size_t N>
struct Comp {
	Comp(bool val = false)
		: value(val) {}
	operator bool() const 
	{ return value; }
	bool value;
};

constexpr size_t N = 100;

using Comp0 = Comp<0>;
using Comp1 = Comp<1>;
using Comp2 = Comp<2>;

using Entity = std::uint32_t;
using CompList = ecfw::type_list<Comp0, Comp1, Comp2>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

using EntityCreatedEvent = ecfw::entity_created<Entity>;
using EntityDestroyedEvent = ecfw::entity_destroyed<Entity>;

using Comp0AddedEvent = ecfw::component_added<Entity, Comp0>;
using Comp1AddedEvent = ecfw::component_added<Entity, Comp1>;
using Comp2AddedEvent = ecfw::component_added<Entity, Comp2>;

using Comp0RemovedEvent = ecfw::component_removed<Entity, Comp0>;
using Comp1RemovedEvent = ecfw::component_removed<Entity, Comp1>;
using Comp2RemovedEvent = ecfw::component_removed<Entity, Comp2>;

struct EntityEventReceiver : ecfw::event_receiver {
	void entity_created(const EntityCreatedEvent& e) const { ASSERT_TRUE(m_manager.valid(e.entity)); }
	void entity_destroyed(const EntityDestroyedEvent& e) { ASSERT_FALSE(m_manager.valid(e.entity)); }

	void comp0_added(const Comp0AddedEvent& e) { ASSERT_TRUE(e.component && m_manager.has<Comp0>(e.entity)); }
	void comp1_added(const Comp1AddedEvent& e) const { ASSERT_TRUE(e.component && m_manager.has<Comp1>(e.entity)); }
	void comp2_added(const Comp2AddedEvent& e) const { ASSERT_TRUE(e.component && m_manager.has<Comp2>(e.entity)); }

	void comp0_removed(const Comp0RemovedEvent& e) { ASSERT_TRUE(e.component && !m_manager.has<Comp0>(e.entity)); }
	void comp1_removed(const Comp1RemovedEvent& e) const { ASSERT_TRUE(e.component && !m_manager.has<Comp1>(e.entity)); }
	void comp2_removed(const Comp2RemovedEvent& e) const { ASSERT_TRUE(e.component && !m_manager.has<Comp2>(e.entity)); }

	EntityEventReceiver(EntityManager& manager)
		: m_manager(manager) {}

	EntityManager& m_manager;
};

TEST(EntityManagerTests, EntityCreationTests) {
	EntityManager manager;

	std::vector<Entity> entities(N);
	manager.create(entities.begin(), entities.end());
	ASSERT_TRUE(std::all_of(entities.begin(), entities.end(), [&manager](auto e) { return manager.valid(e); }));
	ASSERT_EQ(manager.num_entities(), N);

	manager.create<Comp0, Comp1,Comp2>(entities.begin(), entities.end());
	ASSERT_TRUE(std::all_of(entities.begin(), entities.end(), [&manager](auto e) { 
		return manager.valid(e) && manager.has<Comp0, Comp1,Comp2>(e);
	}));
	ASSERT_EQ(manager.num_entities(), 2 * N);
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), N);

	manager.create(N);
	ASSERT_EQ(manager.num_entities(), 3 * N);

	manager.create<Comp0, Comp1, Comp2>(N);
	ASSERT_EQ(manager.num_entities(), 4 * N);
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), 2 * N);

	Entity progenitor = manager.create<Comp0, Comp1, Comp2>();
	ASSERT_TRUE((manager.has<Comp0, Comp1, Comp2>(progenitor)));

	manager.clone<Comp0, Comp1, Comp2>(progenitor, N);
	ASSERT_EQ(manager.num_entities(), 5 * N + 1);
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), 3 * N + 1);

	manager.clone<Comp0, Comp1, Comp2>(progenitor, entities.begin(), entities.end());
	ASSERT_TRUE(std::all_of(entities.begin(), entities.end(), [&manager](auto e) {
		return manager.valid(e) && manager.has<Comp0, Comp1, Comp2>(e);
	}));
	ASSERT_EQ(manager.num_entities(), 6 * N + 1);
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), 4 * N + 1);
	
}

TEST(EntityManagerTests, EntityDestructionTests) {
	EntityManager manager;

	std::vector<Entity> entities(N);
	manager.create<Comp0, Comp1, Comp2>(entities.begin(), entities.end());
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), N);
	ASSERT_EQ(manager.num_entities(), N);
	ASSERT_EQ(manager.num_live_entities(), N);
	ASSERT_EQ(manager.num_recyclable_entities(), 0);
	ASSERT_EQ(manager.num_reusable_entities(), 0);

	manager.destroy(entities.begin(), entities.end());
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), N);
	ASSERT_EQ(manager.num_live_entities(), N);
	ASSERT_EQ(manager.num_recyclable_entities(), N);
	ASSERT_EQ(manager.num_reusable_entities(), 0);
	
	manager.update();

	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), 0);
	ASSERT_EQ(manager.num_live_entities(), 0);
	ASSERT_EQ(manager.num_recyclable_entities(), 0);
	ASSERT_EQ(manager.num_reusable_entities(), N);
}

TEST(EntityManagerTests, EntityReuseTests) {
	EntityManager manager;

	std::vector<Entity> entities(N);
	manager.create<Comp0, Comp1, Comp2>(entities.begin(), entities.end());
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), N);
	ASSERT_EQ(manager.num_entities(), N);
	ASSERT_EQ(manager.num_live_entities(), N);
	ASSERT_EQ(manager.num_recyclable_entities(), 0);
	ASSERT_EQ(manager.num_reusable_entities(), 0);

	manager.destroy(entities.begin(), entities.end());
	ASSERT_EQ(N, (manager.num_entities_with<Comp0, Comp1, Comp2>()));
	ASSERT_EQ(manager.num_live_entities(), N);
	ASSERT_EQ(manager.num_recyclable_entities(), N);
	ASSERT_EQ(manager.num_reusable_entities(), 0);

	manager.update();

	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), 0);
	ASSERT_EQ(manager.num_live_entities(), 0);
	ASSERT_EQ(manager.num_recyclable_entities(), 0);
	ASSERT_EQ(manager.num_reusable_entities(), N);

	manager.create<Comp0, Comp1, Comp2>(entities.begin(), entities.end());
	ASSERT_EQ((manager.num_entities_with<Comp0, Comp1, Comp2>()), N);
	ASSERT_EQ(manager.num_entities(), N);
	ASSERT_EQ(manager.num_live_entities(), N);
	ASSERT_EQ(manager.num_recyclable_entities(), 0);
	ASSERT_EQ(manager.num_reusable_entities(), 0);
}

TEST(EntityManagerTests, ComponentAddingTests) {
	EntityManager manager;

	Entity e = manager.create();

	ASSERT_TRUE(manager.assign<Comp0>(e, true));
	ASSERT_TRUE(manager.assign<Comp1>(e, true));
	ASSERT_TRUE(manager.assign<Comp2>(e, true));

	ASSERT_TRUE((manager.has<Comp0, Comp1, Comp2>(e)));
}

TEST(EntityManagerTests, CompoonentRemovedTests) {
	EntityManager manager;

	Entity e = manager.create();

	ASSERT_TRUE(manager.assign<Comp0>(e, true));
	ASSERT_TRUE(manager.assign<Comp1>(e, true));
	ASSERT_TRUE(manager.assign<Comp2>(e, true));

	ASSERT_TRUE((manager.has<Comp0, Comp1, Comp2>(e)));

	manager.remove<Comp0, Comp1, Comp2>(e);

	ASSERT_FALSE((manager.has<Comp0, Comp1, Comp2>(e)));
}

TEST(EntityManagerTests, ComponentStorageTests) {
	EntityManager manager;

	manager.create<Comp0>(N);
	manager.create<Comp1>(N / 2);

	ASSERT_TRUE(manager.empty<Comp2>());
	ASSERT_EQ(manager.size<Comp0>(), N);
	// Default storage is a vector, therefore there are 
	// N unused components
	ASSERT_EQ(manager.size<Comp1>(), N + N / 2);

	manager.reserve<Comp0>(2 * N);
	manager.reserve<Comp1>(2 * N);
	manager.reserve<Comp2>(2 * N);

	ASSERT_TRUE(manager.empty<Comp2>());
	ASSERT_EQ(manager.size<Comp0>(), N);
	ASSERT_EQ(manager.size<Comp1>(), N + N / 2);
}

TEST(EntityManagerTests, SequentialIterationTests) {
	EntityManager manager;

	std::vector<Entity> entities(N);
	manager.create<Comp0, Comp1, Comp2>(entities.begin(), entities.end());

	ASSERT_FALSE(std::all_of(entities.begin(), entities.end(), [&manager](auto entity) { return manager.component<Comp0>(entity); }));

	size_t count = 0;
	manager.entities_with<Comp0>([&count](Comp0& comp) {
		comp.value = true;
		++count;
	});

	ASSERT_EQ(count, (manager.num_entities_with<Comp0>()));
	ASSERT_TRUE(std::all_of(entities.begin(), entities.end(), [&manager](auto entity) { return manager.component<Comp0>(entity); }));

	ASSERT_FALSE(std::all_of(entities.begin(), entities.end(), [&manager](auto entity) { return manager.component<Comp1>(entity); }));
	count = 0;
	manager.entities_with<Comp1>([&count](Comp1& comp) {
		comp.value = true;
		++count;
	});
	ASSERT_EQ(count, (manager.num_entities_with<Comp1>()));
	ASSERT_TRUE(std::all_of(entities.begin(), entities.end(), [&manager](auto entity) { return manager.component<Comp1>(entity); }));

	ASSERT_FALSE(std::all_of(entities.begin(), entities.end(), [&manager](auto entity) { return manager.component<Comp2>(entity); }));
	count = 0;
	manager.entities_with<Comp2>([&count](Comp2& comp) {
		comp.value = true;
		++count;
	});
	ASSERT_EQ(count, (manager.num_entities_with<Comp2>()));
	ASSERT_TRUE(std::all_of(entities.begin(), entities.end(), [&manager](auto entity) { return manager.component<Comp2>(entity); }));

	manager.entities_with<Comp0, Comp1, Comp2>([&manager](auto e, auto& c0, auto& c1, auto& c2) {
		ASSERT_TRUE(manager.component<Comp0>(e));
		ASSERT_TRUE(manager.component<Comp1>(e));
		ASSERT_TRUE(manager.component<Comp2>(e));
	});

}

TEST(EntityManagerTests, EventSubscriptionTests) {
	EntityManager manager;

	EntityEventReceiver receiver(manager);

	manager.events().subscribe<EntityCreatedEvent>(&receiver, &EntityEventReceiver::entity_created);
	manager.events().subscribe<EntityDestroyedEvent>(&receiver, &EntityEventReceiver::entity_destroyed);
	manager.events().subscribe<Comp0AddedEvent>(&receiver, &EntityEventReceiver::comp0_added);
	manager.events().subscribe<Comp1AddedEvent>(&receiver, &EntityEventReceiver::comp1_added);
	manager.events().subscribe<Comp2AddedEvent>(&receiver, &EntityEventReceiver::comp2_added);

	manager.events().subscribe<Comp0RemovedEvent>(&receiver, &EntityEventReceiver::comp0_removed);
	manager.events().subscribe<Comp1RemovedEvent>(&receiver, &EntityEventReceiver::comp1_removed);
	manager.events().subscribe<Comp2RemovedEvent>(&receiver, &EntityEventReceiver::comp2_removed);

	// Tests reception of entity_created
	Entity e = manager.create();

	manager.assign<Comp0>(e, true);
	manager.assign<Comp1>(e, true);
	manager.assign<Comp2>(e, true);

	// Tests removal of entity components Comp0, Comp1, Comp2
	manager.remove<Comp0, Comp1, Comp2>(e);

	// Tests reception of entity destroyed event
	manager.destroy(e);
	manager.update();
}

TEST(EntityManagerTests, ManagerResetTests) {
	EntityManager manager;

	manager.create<Comp0, Comp1, Comp2>(N);
	ASSERT_EQ(manager.num_live_entities(), N);
	ASSERT_EQ(manager.num_entities(), N);
	ASSERT_EQ(manager.size<Comp0>(), N);
	ASSERT_EQ(manager.size<Comp1>(), N);
	ASSERT_EQ(manager.size<Comp2>(), N);

	manager.reset();

	ASSERT_EQ(manager.num_live_entities(), 0);
	ASSERT_EQ(manager.num_entities(), 0);
	ASSERT_EQ(manager.size<Comp0>(), 0);
	ASSERT_EQ(manager.size<Comp1>(), 0);
	ASSERT_EQ(manager.size<Comp2>(), 0);
}