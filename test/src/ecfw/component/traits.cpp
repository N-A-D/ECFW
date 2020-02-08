#include <gtest/gtest.h>
#include <ecfw/component/traits.hpp>
#include <type_traits>

struct Component {
	int x{};
};

template <typename T>
struct AltStorage {
	std::vector<T> data{};
};

namespace ecfw {
	template <>
	struct storage<Component>
	{
		using type = AltStorage<Component>;
	};
}

TEST(ComponentTraitTests, AlternativeStorageContainer) {
	ASSERT_TRUE((std::is_same_v<ecfw::storage_t<Component>, AltStorage<Component>>));
}