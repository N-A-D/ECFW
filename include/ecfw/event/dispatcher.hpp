#pragma once

#include <tuple>
#include <functional>
#include <ecfw/event/signal.hpp>
#include <ecfw/entity/events.hpp>
#include <ecfw/meta/type_list.hpp>
#include <ecfw/utility/noncopyable.hpp>

namespace ecfw {

	template <
		class EventList
	> class event_dispatcher;

	template <
		class... Events
	> class event_dispatcher<type_list<Events...>> : detail::noncopyable {
	public:

		static_assert(meta::is_unique_v<type_list<Events...>>,
			"Duplicate event type(s)!");

		template <
			class Event
		> event_subscription
			subscribe(std::function<void(const Event&)> func) {
			return signal<Event>().connect(func);
		}

		template <
			class Event,
			class Receiver
		> event_subscription
			subscribe(Receiver* receiver, void(Receiver::*func)(const Event&)) {
			static_assert(std::is_base_of_v<event_receiver, Receiver>, 
				"Receiver types must inherit from ecfw::event_receiver");
			return signal<Event>().connect(receiver, func);
		}

		template <
			class Event,
			class Receiver
		> event_subscription
			subscribe(Receiver* receiver, void(Receiver::*func)(const Event&) const) {
			static_assert(std::is_base_of_v<event_receiver, Receiver>,
				"Receiver types must inherit from ecfw::event_receiver");
			return signal<Event>().connect(receiver, func);
		}

		template <
			class Event
		> void operator()(const Event& event) {
			publish<Event>(event);
		}

		template <
			class Event
		> void publish(const Event& event) {
			signal<Event>().emit(event);
		}

	private:

		template <class Event>
		event_signal<Event>& signal() {
			return const_cast<event_signal<Event>&>(
				const_cast<const event_dispatcher*>(this)->signal<Event>());
		}

		template <class Event>
		const event_signal<Event>& signal() const {
			static_assert(meta::contains_v<Event, type_list<Events...>>);
			return std::get<event_signal<Event>>(m_signals);
		}

		std::tuple<event_signal<Events>...> m_signals;
	};

}