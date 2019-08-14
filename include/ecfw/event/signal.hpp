#pragma once

#include <proto/proto.hpp>

namespace ecfw {

	template <
		class Event
	> using event_signal = proto::signal<void(const Event&)>;
	
	using event_receiver = proto::receiver;
	using event_connection = proto::connection;
	using scoped_event_connection = proto::scoped_connection;

}