#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

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

    bool b = false;
    EXPECT_FALSE(b);

    b &= true;
    EXPECT_FALSE(b);

    b |= true;
    EXPECT_TRUE(b);

    std::unordered_map<int, int> m = {
        {1, 2},
        {3, 4}
    };

    EXPECT_EQ(m[1], 2);
    EXPECT_EQ(m[5], 0);
    EXPECT_EQ(m.at(3), 4);
    EXPECT_THROW(m.at(7), std::exception);
}

TEST(HelloTest, PrintChar)
{
    char          a = 99;
    unsigned char b = 99;
    EXPECT_EQ(a, b);

    a = 128;
    b = 128;
    EXPECT_NE(a, b);  // not equal

    a = 200;
    b = 200;
    EXPECT_NE(a, b);  // not equal
}

TEST(HelloTest, UniquePtr)
{
    class B
    {
    public:
        B()
        {
            std::cout << "B::B" << std::endl;
        }

        ~B()
        {
            std::cout << "B::~B" << std::endl;
        }
    };

    std::unique_ptr<B> p;
    std::cout << (p == nullptr) << std::endl;

    std::cout << "creating p" << std::endl;
    p = std::make_unique<B>();

    struct
    {
        std::unique_ptr<B> b;
    } c;

    std::cout << "moving p to c.b" << std::endl;
    c.b = std::move(p);

    std::cout << "resetting c" << std::endl;
    c = {0};
}

TEST(HelloTest, CharVectorManip)
{
    char l[] = "abcdefghijklmnopqrstuvwxyz";

    std::cout << "l = " << l << ", strlen(l) = " << strlen(l) << ", sizeof(l) = " << sizeof(l) << std::endl;

    std::vector<char> v(l, l + sizeof(l));

    std::cout << "v = " << v.data() << ", v.size() = " << v.size() << std::endl;

    int x = 3456;

    v.insert(v.begin(), x);
    v.insert(v.begin(), x >> 8);
    v.insert(v.begin(), x >> 16);
    v.insert(v.begin(), x >> 24);
    v.insert(v.begin(), 'Q');

    std::cout << "v = " << v.data() + 5 << ", v.size() = " << v.size() << std::endl;

    std::vector<char> w = std::move(v);

    std::cout << "v.size() = " << v.size() << ", v.capacity() = " << v.capacity() << std::endl;
    std::cout << "w = " << w.data() + 5 << ", w.size() = " << w.size() << ", w.capacity() = " << w.capacity()
              << std::endl;

    // ROUND 2

    std::cout << "-------------------------------" << std::endl;

    char* l_ = (char*)malloc(sizeof(l));
    memcpy(l_, l, sizeof(l));

    std::cout << "l_ = " << l_ << ", strlen(l_) = " << strlen(l_) << ", sizeof(l_) = " << sizeof(l_) << std::endl;

    std::vector<char> v_(l_, l_ + strlen(l_) + 1);
    free(l_);

    std::cout << "v_ = " << v_.data() << ", v_.size() = " << v_.size() << ", v_.capacity() = " << v_.capacity()
              << std::endl;

    class B
    {
    public:
        std::vector<char> d;

        void Take(std::vector<char>& in)
        {
            d = std::move(in);
        }
    };

    B b;
    b.Take(v_);

    std::cout << "v_.size() = " << v_.size() << ", v_.capacity() = " << v_.capacity() << std::endl;

    std::cout << "b.d = " << b.d.data() << ", b.d.size() = " << b.d.size() << ", b.d.capacity() = " << b.d.capacity()
              << std::endl;
}

TEST(HelloTest, IntToBytesViceVersa)
{
    int x = 3456;

    char data[sizeof x] = {0};

    data[3] = x;
    data[2] = x >> 8;
    data[1] = x >> 16;
    data[0] = x >> 24;

    int y = 0;

    y += int((unsigned char)data[0]) << 24;
    y += int((unsigned char)data[1]) << 16;
    y += int((unsigned char)data[2]) << 8;
    y += int((unsigned char)data[3]);

    EXPECT_EQ(x, y);
}

TEST(HelloTest, VectorAsResizableBuffer)
{
    std::size_t       buf_size  = 4;
    std::size_t       prev_size = 0;
    std::vector<char> buf(buf_size * 2);

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
    ASSERT_THAT(buf, ElementsAre('a', 'a', 'a', 'a', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', '\0', '\0', '\0', '\0'));

    // c
    prev_size += buf_size;
    buf_size *= 2;
    buf.resize(buf_size * 2);
    std::memcpy(buf.data() + prev_size, std::string(buf_size, 'c').data(), buf_size);
    EXPECT_EQ(32, buf.size());
    ASSERT_THAT(buf, ElementsAre('a', 'a', 'a', 'a', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'c', 'c', 'c', 'c', 'c',
                                 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c', '\0', '\0', '\0', '\0'));
}

TEST(HelloTest, JsonExamples)
{
    // json root = {
    //     {"stmts",
    //      {{{"stmt", {{"DropStmt", {{"objects", {{{"List", {{"items", {{{"String", {{"str", "zzz"}}}}}}}}}}}}}}}}}}
    // };

    json root = {
        {"stmts", {{{"stmt", {{"DropStmt", {{"objects", {}}}}}}}}}
    };

    std::cout << root.dump(4) << "\n---------------" << std::endl;

    json copy(root);
    std::cout << "step 1\n";

    json& objs = copy["stmts"][0]["stmt"]["DropStmt"]["objects"];
    std::cout << "step 2\n";

    objs.emplace_back(json({
        {"List", {{"items", {{{"String", {{"str", "xxx"}}}}}}}}
    }));
    std::cout << "step 3\n";

    objs.emplace_back(json({
        {"List", {{"items", {{{"String", {{"str", "yyy"}}}}}}}}
    }));
    std::cout << "step 4\n";

    std::cout << copy.dump(4) << "\n---------------" << std::endl;

    std::cout << root.dump(4) << "\n---------------" << std::endl;
}
}  // namespace gmock_matchers_test
}  // namespace testing
