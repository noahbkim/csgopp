#pragma once

#include <array>
#include <vector>
#include <stdexcept>

namespace csgopp::common::ring
{

// Marker to use vector
constexpr size_t Runtime = 0;

template<typename T, size_t Capacity>
struct Container
{
public:
    using Data = std::array<T, Capacity>;
    using Index = size_t;
    Container() : _data() {}

protected:
    Data _data;
    Index _start{0};
    Index _size{0};

    [[nodiscard]] inline constexpr Index capacity() const
    {
        return Capacity;
    }
};

template<typename T>
struct Container<T, Runtime>
{
public:
    using Data = std::vector<T>;
    using Index = typename Data::size_type;
    explicit Container(size_t size) : _data(size) {}

protected:
    Data _data;
    Index _start{0};
    Index _size{0};

    [[nodiscard]] inline Index capacity() const
    {
        return this->_data.size();
    }
};

template<typename T, size_t Capacity = Runtime>
class Ring : public Container<T, Capacity>
{
public:
    using Data = typename Container<T, Capacity>::Data;
    using Index = typename Container<T, Capacity>::Index;

    template<typename... Args>
    explicit Ring(Args... args) : Container<T, Capacity>(args...) {}

    void push_back(const T& item)
    {
        if (this->_size >= this->capacity())
        {
            throw std::runtime_error("ring is at capacity!");
        }

        this->_data.at(this->absolute(this->_size)) = item;
        this->_size += 1;
    }

    inline void push_back_overwrite(const T& item)
    {
        this->_data.at(this->absolute(this->_size)) = item;
        if (this->_size < this->capacity())
        {
            this->_size += 1;
        }
        else
        {
            this->_start += 1;
        }
    }

    void pop_front()
    {
        this->_start = this->absolute(1);
        this->_size -= 1;
    }

    T& at(Index index)
    {
        return this->_data.at(this->absolute(index));
    }

    const T& at(Index index) const
    {
        return this->_data.at(this->absolute(index));
    }

    Index size() const
    {
        return this->_size;
    }

private:
    [[nodiscard]] inline Index absolute(Index external) const
    {
        return (this->_start + external) % this->capacity();
    }
};

}
