#include "common/error.h"

#include <gtest/gtest.h>

#include <iostream>
#include <utility>

class Foo
{
private:
    int         x_;
    std::string y_;

public:
    Foo()
    {
        std::cout << "Foo: default constructor\n";
    }

    Foo(const Foo& d)
    {
        std::cout << "Foo: copy constructor\n";
    }

    Foo(Foo&& d) noexcept
    {
        std::cout << "Foo: move constructor\n";
    }

    ~Foo()
    {
        std::cout << "Foo: destructor\n";
    }
};

class Bar
{
private:
    int         x_;
    std::string y_;

public:
    Bar()
    {
        std::cout << "Bar: default constructor\n";
    }

    Bar(const Bar& d)
    {
        std::cout << "Bar: copy constructor\n";
    }

    Bar(Bar&& d) noexcept
    {
        std::cout << "Bar: move constructor\n";
    }

    ~Bar()
    {
        std::cout << "Bar: destructor\n";
    }
};

std::pair<Foo, Bar> FooBar()
{
    Foo f;
    Bar b;
    return {std::move(f), std::move(b)};
}

std::pair<Error, Error> ErrErr()
{
    Error e1;
    Error e2;
    return {std::move(e1), std::move(e2)};
}

TEST(ErrorTest, ReturnTuple)
{
    std::cout << "before FooBar\n";
    auto [f, b] = FooBar();
    std::cout << "after FooBar\n";

    std::cout << "before ErrErr\n";
    auto [e1, e2] = ErrErr();
    std::cout << "after ErrErr\n";
}
