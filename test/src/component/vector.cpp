#include <gtest/gtest.h>
#include <memory>
#include <array>
#include <numeric>
#include <algorithm>
#include <random>
#include <ecfw/component/vector.hpp>

struct Element {
	Element(int id, std::shared_ptr<bool> state_ref = nullptr)
		: id{id},
		m_state_ref{ state_ref }
	{}
	operator bool() const { return *m_state_ref; }
	int id{};
	std::shared_ptr<bool> m_state_ref{};
};

TEST(PageVectorTests, DefaultBlockSizeInterfaceTests) {
	using buffer_t0 = ecfw::block_vector<Element>;
	std::shared_ptr<bool> state = std::make_shared<bool>(true);
	const int limit = 65536;

	// Linear interface tests

	// Test element construction
	buffer_t0 buffer0{};
	
	for (auto i = 0; i != limit; ++i) {
		auto element = buffer0.construct(i, i, state);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, i);
		ASSERT_EQ(state.use_count(), element->m_state_ref.use_count());
	}
	ASSERT_EQ(state.use_count(), limit + 1);
	
	// Test element access
	for (auto i = 0; i != limit; ++i) {
		auto element = buffer0.at(i);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, i);
		ASSERT_EQ(element->m_state_ref.use_count(), state.use_count());
	}

	// Test the container size
	ASSERT_EQ(buffer0.size(), limit);

	// Test element destruction
	for (auto i = 0; i != limit; ++i) {
		buffer0.destroy(i);
		ASSERT_EQ(state.use_count(), limit - i);
	}
	ASSERT_EQ(state.use_count(), 1);

	// Nonlinear interface tests

	std::vector<int> positions(limit);
	std::iota(positions.begin(), positions.end(), 0);
	const int step = limit / 4;

	auto it = positions.begin();
	for (int i = 0; i != 4; ++i) {
		if (i % 2 == 1)
			std::reverse(it, it + step);
		it += step;
	}

	buffer_t0 buffer1{};

	// Test element construction with unordered positions
	for (auto position : positions) {
		auto element = buffer1.construct(position, position, state);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, position);
		ASSERT_EQ(state.use_count(), element->m_state_ref.use_count());
	}
	ASSERT_EQ(state.use_count(), limit + 1);

	// Test element access with unordered positions
	for (auto position : positions) {
		auto element = buffer1.at(position);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, position);
		ASSERT_EQ(element->m_state_ref.use_count(), state.use_count());
	}

	// Test the container size
	ASSERT_EQ(buffer1.size(), limit);

	// Test element destruction using  random positions
	for (auto position :  positions) {
		buffer1.destroy(position);
	}
	ASSERT_EQ(state.use_count(), 1);
}

TEST(PageVectorTests, CustomBlockSizeInterfaceTests) {
	using buffer_t0 = ecfw::block_vector<Element, 1337>;
	std::shared_ptr<bool> state = std::make_shared<bool>(true);
	const int limit = 65536;
	const int block_size = 1337;
	// Linear interface tests

	// Test element construction
	buffer_t0 buffer0{};

	for (auto i = 0; i != limit; ++i) {
		auto element = buffer0.construct(i, i, state);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, i);
		ASSERT_EQ(state.use_count(), element->m_state_ref.use_count());
	}
	ASSERT_EQ(state.use_count(), limit + 1);

	// Test element access
	for (auto i = 0; i != limit; ++i) {
		auto element = buffer0.at(i);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, i);
		ASSERT_EQ(element->m_state_ref.use_count(), state.use_count());
	}

	ASSERT_EQ(buffer0.size(), (limit / block_size + 1) * block_size);

	// Test element destruction
	for (auto i = 0; i != limit; ++i) {
		buffer0.destroy(i);
		ASSERT_EQ(state.use_count(), limit - i);
	}
	ASSERT_EQ(state.use_count(), 1);

	// Nonlinear interface tests

	std::vector<int> positions(limit);
	std::iota(positions.begin(), positions.end(), 0);
	const int step = limit / 4;

	auto it = positions.begin();
	for (int i = 0; i != 4; ++i) {
		if (i % 2 == 1)
			std::reverse(it, it + step);
		it += step;
	}

	buffer_t0 buffer1{};

	// Test element construction with unordered positions
	for (auto position : positions) {
		auto element = buffer1.construct(position, position, state);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, position);
		ASSERT_EQ(state.use_count(), element->m_state_ref.use_count());
	}
	ASSERT_EQ(state.use_count(), limit + 1);

	// Test element access with unordered positions
	for (auto position : positions) {
		auto element = buffer1.at(position);
		ASSERT_TRUE(element);
		ASSERT_TRUE((*element));
		ASSERT_EQ(element->id, position);
		ASSERT_EQ(element->m_state_ref.use_count(), state.use_count());
	}

	// Test the container size
	ASSERT_EQ(buffer1.size(), (limit / block_size +  1) * block_size);

	// Test element destruction using  random positions
	for (auto position : positions) {
		buffer1.destroy(position);
	}
	ASSERT_EQ(state.use_count(), 1);
}
