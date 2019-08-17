#include <gtest/gtest.h>
#include <ecfw/event/dispatcher.hpp>

template <size_t N>
struct Event {
	bool value;
};

using Event0 = Event<0>;
using Event1 = Event<1>;

struct Receiver0 : ecfw::event_receiver {
	void func0(const Event0& e0) 
	{ ASSERT_TRUE(e0.value); }

	void func1(const Event1& e1) const 
	{ ASSERT_TRUE(e1.value); }
};

void free_func0(const Event0& e0) {
	ASSERT_TRUE(e0.value);
}

void free_func1(const Event1& e1) {
	ASSERT_TRUE(e1.value);
}

using EventList = ecfw::type_list<Event0, Event1>;
using EventDispatcher = ecfw::event_dispatcher<EventList>;

TEST(EventDispatcherTests, EventSubscriptionTests) {
	auto func0 = [](const Event0& e0) { ASSERT_TRUE(e0.value); };
	auto func1 = [](const Event1& e1) { ASSERT_TRUE(e1.value); };

	EventDispatcher dispatcher;

	ASSERT_TRUE(dispatcher.subscribe<Event0>(func0));
	ASSERT_TRUE(dispatcher.subscribe<Event1>(func1));

	Receiver0 receiver;
	dispatcher.subscribe<Event0>(&receiver, &Receiver0::func0);
	dispatcher.subscribe<Event1>(&receiver, &Receiver0::func1);

	ASSERT_EQ(receiver.num_connections(), 2);

	ASSERT_TRUE(dispatcher.subscribe<Event0>(free_func0));
	ASSERT_TRUE(dispatcher.subscribe<Event0>(free_func0));
}

TEST(EventDispatcherTests, EventPublishTests) {
	auto func0 = [](const Event0& e0) { ASSERT_TRUE(e0.value); };
	auto func1 = [](const Event1& e1) { ASSERT_TRUE(e1.value); };

	EventDispatcher dispatcher;

	ASSERT_TRUE(dispatcher.subscribe<Event0>(func0));
	ASSERT_TRUE(dispatcher.subscribe<Event1>(func1));

	Receiver0 receiver;

	dispatcher.subscribe<Event0>(&receiver, &Receiver0::func0);
	dispatcher.subscribe<Event1>(&receiver, &Receiver0::func1);

	ASSERT_EQ(receiver.num_connections(), 2);

	ASSERT_TRUE(dispatcher.subscribe<Event0>(free_func0));
	ASSERT_TRUE(dispatcher.subscribe<Event0>(free_func0));

	dispatcher(Event0{ true });
	dispatcher(Event1{ true });

	dispatcher.publish(Event0{ true });
	dispatcher.publish(Event1{ true });
}