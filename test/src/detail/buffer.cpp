#include <gtest/gtest.h>
#include <ecfw/detail/buffer.hpp>

using namespace ecfw::detail;

TEST(buffer, make_buffer) {
    buffer_ptr ptr = make_buffer<int>();
    ASSERT_TRUE(ptr);
    ptr.reset();
    ASSERT_FALSE(ptr);
}