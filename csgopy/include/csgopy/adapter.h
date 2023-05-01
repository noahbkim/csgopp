#pragma once

#include <memory>

template<typename T>
struct Adapter
{
    std::shared_ptr<T> self;

    Adapter() = default;
    explicit Adapter(std::shared_ptr<T> self) : self(std::move(self)) {}
};
