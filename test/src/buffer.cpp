#include <random>
#include <memory>
#include <algorithm>
#include <execution>
#include <unordered_map>
#include <gtest/gtest.h>
#include <ecfw/detail/buffer.hpp>
#include "counter.hpp"

namespace dtl = ecfw::detail;

class Object : public Counter<Object> {
public:

	Object()
		: Counter<Object>()
		, iden(how_many())
	{}

	size_t id() const noexcept {
		return iden;
	}

private:
	size_t iden{};
};

TEST(buffer, chunked_buffer)
{
	
	const size_t block_size = 2;
	std::vector<size_t> indices = { 7, 8, 1, 2, 5, 6, 3, 4 };
	std::vector<std::pair<size_t, size_t>> indices_to_identities{};
	dtl::chunked_buffer buffer(sizeof(Object), 8192, &dtl::destroy_object<Object>);

	// Test construction
	for (auto index : indices)
	{
		buffer.accommodate(index);
		Object* data = static_cast<Object*>(buffer.data(index));
		buffer.construct(data);
		indices_to_identities.emplace_back(index, data->id());
	}
	ASSERT_EQ(Object::how_many(), indices.size());

	std::mt19937 gen{ std::random_device{}() };
	std::shuffle(indices_to_identities.begin(), indices_to_identities.end(), gen);

	// Test element access
	for (auto [index, id] : indices_to_identities) {
		Object* data = static_cast<Object*>(buffer.data(index));
		ASSERT_EQ(data->id(), id);
	}

	// Test destruction
	for (auto index : indices) {
		auto p = buffer.data(index);
		buffer.destroy(p);
	}
	ASSERT_EQ(Object::how_many(), 0);

}