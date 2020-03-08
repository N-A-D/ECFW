#include <gtest/gtest.h>
#include <memory>
#include <ecfw/component/vector.hpp>

struct Element {
	Element(std::shared_ptr<bool> state_ref = nullptr)
		: m_state_ref{ state_ref }
	{}
	operator bool() const { return *m_state_ref; }
	std::shared_ptr<bool> m_state_ref{};
};

using buffer_t = ecfw::paged_vector<Element>;

TEST(PagedVectorTests, FunctionalityTests) {
	std::shared_ptr<bool> state = std::make_shared<bool>(true);
	// Test element construction
	buffer_t buffer{};
	const int limit = 65536;
	for (auto i = 0; i != limit; ++i) {
		auto element = buffer.construct(i, state);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(state.use_count(), i + 2);
	}
	ASSERT_EQ(state.use_count(), limit + 1);
	
	// Test element access
	for (auto i = 0; i != limit; ++i) {
		auto element = buffer.at(i);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->m_state_ref.use_count(), state.use_count());
	}

	// Test the container size
	ASSERT_EQ(buffer.size(), limit);

	// Test element destruction
	for (auto i = 0; i != limit; ++i) {
		buffer.destroy(i);
		ASSERT_EQ(state.use_count(), limit - i);
	}
	ASSERT_EQ(state.use_count(), 1);
}
