#pragma once

#include <stdexcept>
#include <string>
#include <istream>
#include <type_traits>
#include <cstddef>

#include "../error.h"

namespace csgopp::common::reader
{

class ReadError : public csgopp::error::Error
{
    using Error::Error;
};

class ReadEOF : public ReadError
{
    using ReadError::ReadError;
};

class ReadFailure : public ReadError
{
    using ReadError::ReadError;
};

struct LittleEndian
{
    template<typename T>
    static constexpr size_t reorder(size_t index)
    {
        return index;
    }
};

struct BigEndian
{
    template<typename T>
    static constexpr size_t reorder(size_t index)
    {
        return sizeof(T) - index - 1;
    }
};

/// Universal interface for streaming bytes
class Reader
{
public:
    template<typename T>
    T deserialize()
    {
        T result;
        this->consume(reinterpret_cast<char*>(&result), sizeof(T));
        return result;
    }

    template<typename T, typename Order>
    T deserialize()
    {
        T result;
        for (size_t i{0}; i < sizeof(T); ++i)
        {
            size_t j = (Order::template reorder<T>(i));
            this->consume(reinterpret_cast<char*>(&result) + j, 1);
        }
        return result;
    }

    template<typename T>
    void read(T* buffer, size_t count)
    {
        this->consume(reinterpret_cast<char*>(buffer), count * sizeof(T));
    }

    template<typename T>
    T container(size_t size)
    {
        T result;
        result.resize(size);
        this->read(result.data(), size);
        return result;
    }

    std::string terminated()
    {
        std::string result;
        do
        {
            result.push_back(0);
            this->read(&result.back(), 1);
        } while (result.back() != 0);
        result.pop_back();
        return result;
    }

    std::string terminated(size_t max)
    {
        std::string result;
        result.reserve(max);
        size_t i = 0;
        do
        {
            result.push_back(0);
            this->read(&result.back(), 1);
            i += 1;
        } while (result.back() != 0 && i < max);
        result.pop_back();

        if (i < max)
        {
            this->skip(max - i);
        }

        return result;
    }

    virtual void skip(size_t size) = 0;
    virtual size_t tell() = 0;

protected:
    virtual void consume(char* into, size_t size) = 0;
};

/// Wrap istream
template<typename Stream>
class StreamReader final : public Reader
{
public:
    template<typename... Args>
    explicit StreamReader(Args&& ...args) : stream(args...)
    {
    }

    void skip(size_t size) override
    {
        this->stream.seekg(size, std::ios::cur);
    }

    [[nodiscard]] size_t tell() override
    {
        return this->stream.tellg();
    }

    void consume(char* into, size_t size) override
    {
        this->stream.read(into, size);
        if (this->stream.eof())
        {
            throw ReadEOF("encountered EOF while reading " + std::to_string(size) + " bytes!");
        }
        if (this->stream.fail())
        {
            throw ReadFailure("failed while reading " + std::to_string(size) + " bytes!");
        }
    }

private:
    static_assert(std::is_base_of<std::istream, Stream>::value);
    Stream stream;
};

template<typename T>
class ContainerReader final : public Reader
{
public:
    explicit ContainerReader(const T& data) : data(data)
    {
    }

    void skip(size_t size) override
    {
        this->cursor += size;
    }

    [[nodiscard]] size_t tell() override
    {
        return this->cursor;
    }

    void consume(char* into, size_t size) override
    {
        if (this->cursor + size > this->data.size())
        {
            throw ReadEOF("encountered EOF while reading " + std::to_string(size) + " bytes!");
        }
        memcpy(into, &this->data.at(this->cursor), size);
        this->cursor += size;
    }

private:
    static_assert(sizeof(typename T::value_type) == 1);

    const T& data;
    size_t cursor{0};
};

}
