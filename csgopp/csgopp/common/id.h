#pragma once

#include <functional>

namespace csgopp::common::id
{

template<typename Discriminator, typename Value = size_t>
class IdBase
{
public:
    using value_type = Value;

    IdBase() noexcept: value(autoincrement())
    {}

    IdBase(const IdBase& other) noexcept: value(other.value)
    {}

    IdBase& operator=(const IdBase& other)
    {
        this->value = other.value;
        return *this;
    }

    explicit operator value_type() const
    {
        return this->value;
    }

    bool operator==(const IdBase& other) const
    {
        return this->value == other.value;
    }

    bool operator!=(const IdBase& other) const
    {
        return this->value != other.value;
    }

private:
    value_type value;

    static value_type autoincrement()
    {
        static value_type index = 0;
        return index++;
    }
};

}

template<typename Discriminator, typename Value>
struct std::hash<csgopp::common::id::IdBase<Discriminator, Value>>
{
    std::size_t operator()(const csgopp::common::id::IdBase<Discriminator, Value>& id) const noexcept
    {
        return static_cast<std::size_t>(id);
    }
};
