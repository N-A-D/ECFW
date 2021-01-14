#include <array>
#include <random>
#include <numeric>
#include <algorithm>
#include <gtest/gtest.h>
#include <ecfw/detail/sparse_set.hpp>

using sparse_set = ecfw::detail::sparse_set<std::uint32_t>;

struct SparseSetTest : public ::testing::Test {
    void SetUp() override {
        std::iota(data.begin(), data.end(), 0);
        std::reverse(data.begin(), data.end());
        auto insert_into_set = [this](auto i) { set.insert(i); };
        std::for_each(data.begin(), data.end(), insert_into_set);
    }
protected:
    sparse_set set{};
    std::array<uint64_t, 16384> data{};
};

TEST_F(SparseSetTest, move_constructor)
{
    sparse_set other{std::move(set)};
    ASSERT_TRUE(set.empty());
    auto contained_by_other = [&other](auto i){ return other.contains(i); };
    ASSERT_TRUE(std::all_of(data.begin(), data.end(), contained_by_other));
}

TEST_F(SparseSetTest, move_assignment)
{
    sparse_set other = std::move(set);
    ASSERT_TRUE(set.empty());
    auto contained_by_other = [&other](auto i){ return other.contains(i); };
    ASSERT_TRUE(std::all_of(data.begin(), data.end(), contained_by_other));
}

TEST_F(SparseSetTest, insert_new_elements) {
    std::array<uint64_t, 16384> new_data{};
    std::iota(new_data.begin(), new_data.end(), uint64_t(data.size()));
    auto contained_by_set = [this](auto i) { return set.contains(i); };
    ASSERT_TRUE(
        std::none_of(new_data.begin(), new_data.end(), contained_by_set));
    auto insert_into_set = [this](auto i) { set.insert(i); };
    std::for_each(new_data.begin(), new_data.end(), insert_into_set);
    ASSERT_TRUE(
        std::all_of(new_data.begin(), new_data.end(), contained_by_set));
    ASSERT_TRUE(
        std::all_of(data.begin(), data.end(), contained_by_set));
}

TEST_F(SparseSetTest, try_insert_existing_elements)
{
    auto insert_into_set = [this](auto i) { set.insert(i); };
    std::for_each(data.begin(), data.end(), insert_into_set);
    ASSERT_EQ(set.size(), data.size());
    auto contained_by_set = [this] (auto i) { return set.contains(i); };
    ASSERT_TRUE(std::all_of(data.begin(), data.end(), contained_by_set));
}

TEST_F(SparseSetTest, remove_existing_elements)
{
    std::reverse(data.begin(), data.end());
    auto remove_from_set = [this](auto i) { set.erase(i); };
    std::for_each(data.begin(), data.end(), remove_from_set);
    auto contained_by_set = [this](auto i) { return set.contains(i); };
    ASSERT_TRUE(std::none_of(data.begin(), data.end(), contained_by_set));
}

TEST_F(SparseSetTest, remove_nonexisting_elements)
{
    std::array<uint64_t, 16384> new_data{};
    std::iota(new_data.begin(), new_data.end(), uint64_t(data.size()));
    auto contained_by_set = [this](auto i) { return set.contains(i); };
    ASSERT_TRUE(
        std::none_of(new_data.begin(), new_data.end(), contained_by_set));
    auto remove_from_set = [this](auto i) { set.erase(i); };
    std::for_each(new_data.begin(), new_data.end(), remove_from_set);
    ASSERT_TRUE(std::all_of(data.begin(), data.end(), contained_by_set));
    ASSERT_TRUE(
        std::none_of(new_data.begin(), new_data.end(), contained_by_set));
}

TEST_F(SparseSetTest, iterator_pre_increment) {
    auto control = data.begin();
    auto candidate = set.begin();

    ++control;
    ++candidate;

    ASSERT_EQ(*control, *candidate);
}

TEST_F(SparseSetTest, iterator_post_increment) {
    auto control = data.begin();
    auto candidate = set.begin();

    control++;
    candidate++;

    ASSERT_EQ(*control, *candidate);
}

TEST_F(SparseSetTest, iterator_pre_decrement)
{
    auto control = data.end();
    auto candidate = set.end();

    --control;
    --candidate;

    ASSERT_EQ(*control, *candidate);
}

TEST_F(SparseSetTest, iterator_post_decrement) {
    ASSERT_EQ(set.begin()--, set.begin());
    ASSERT_EQ(set.end()--, set.end());
}

TEST_F(SparseSetTest, iterator_increment)
{
    auto control = data.begin();
    auto candidate = set.begin();

    control += 1;
    candidate += 1;

    ASSERT_EQ(*control, *candidate);
}

TEST_F(SparseSetTest, iterator_decrement)
{
    auto control = data.end();
    auto candidate = set.end();

    control -= 1;
    candidate -= 1;

    ASSERT_EQ(*control, *candidate);
}

TEST_F(SparseSetTest, iterator_scalar_addition) {
	auto control = data.begin();
	auto candidate = set.begin();

	ASSERT_EQ(*(control + 1), *(candidate + 1));
}

TEST_F(SparseSetTest, iterator_scalar_subtraction) {
	auto control = data.end();
	auto candidate = set.end();

	ASSERT_EQ(*(control - 1), *(candidate - 1));
}

TEST_F(SparseSetTest, iterator_equality) {
	ASSERT_EQ(set.begin(), set.begin());
	ASSERT_EQ(set.end(), set.end());
}

TEST_F(SparseSetTest, iterator_not_equal) {
	ASSERT_NE(set.begin(), set.end());
    ASSERT_NE(set.begin(), set.begin() + 1);
}

TEST_F(SparseSetTest, iterator_less_than) {
	ASSERT_LT(set.begin(), set.end());
    ASSERT_LT(set.begin() + 1, set.end());
}

TEST_F(SparseSetTest, iteraor_less_than_or_equal_to) {
	ASSERT_LE(set.begin(), set.begin());
	ASSERT_LE(set.begin(), set.begin() + 1);
	ASSERT_LE(set.begin(), set.end());
}

TEST_F(SparseSetTest, iterator_greater_than) {
	ASSERT_GT(set.end(), set.begin());
	ASSERT_GT(set.end(), set.end() - 1);
}

TEST_F(SparseSetTest, iterator_greater_than_or_equal_to) {
	ASSERT_GE(set.end(), set.end());
	ASSERT_GE(set.end(), set.begin());
	ASSERT_GE(set.end(), set.end() - 1);
}
