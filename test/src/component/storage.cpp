#include <gtest/gtest.h>
#include <type_traits>
#include <ecfw/component/storage.hpp>

struct Component0 {};

struct Component1 {};

template <typename T>
struct AltStorage {};

namespace ecfw {
	template <>
	struct storage<Component1> {
		using type = AltStorage<Component1>;
	};
}

TEST(ComponentStorageTests, StorageDefinitionTests) {
	ASSERT_TRUE((std::is_same_v<ecfw::storage_t<Component0>, ecfw::paged_vector<Component0>>));
	ASSERT_FALSE((std::is_same_v<ecfw::storage_t<Component1>, ecfw::paged_vector<Component1>>));
	ASSERT_TRUE((std::is_same_v<ecfw::storage_t<Component1>, AltStorage<Component1>>));
}