#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace testing
{
namespace gmock_matchers_test
{

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

TEST(HelloTest, VectorAsResizableBuffer)
{
    std::size_t                buf_size  = 4;
    std::size_t                prev_size = 0;
    std::vector<unsigned char> buf(buf_size * 2);

    // a
    std::memcpy(buf.data(), std::string(buf_size, 'a').data(), buf_size);
    EXPECT_EQ(8, buf.size());
    ASSERT_THAT(buf, ElementsAre('a', 'a', 'a', 'a', '\0', '\0', '\0', '\0'));

    // b
    prev_size += buf_size;
    buf_size *= 2;
    buf.resize(buf_size * 2);
    std::memcpy(buf.data() + prev_size, std::string(buf_size, 'b').data(), buf_size);
    EXPECT_EQ(16, buf.size());
    ASSERT_THAT(buf, ElementsAre('a', 'a', 'a', 'a', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', '\0',
                                 '\0', '\0', '\0'));

    // c
    prev_size += buf_size;
    buf_size *= 2;
    buf.resize(buf_size * 2);
    std::memcpy(buf.data() + prev_size, std::string(buf_size, 'c').data(), buf_size);
    EXPECT_EQ(32, buf.size());
    ASSERT_THAT(buf, ElementsAre('a', 'a', 'a', 'a', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'c',
                                 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c',
                                 'c', 'c', '\0', '\0', '\0', '\0'));
}
}  // namespace gmock_matchers_test
}  // namespace testing
