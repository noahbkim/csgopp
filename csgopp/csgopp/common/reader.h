#pragma once

#include <stdexcept>
#include <string>
#include <istream>
#include <type_traits>
#include <cstddef>

#include "../error.h"

namespace csgopp::common::reader
{

using digit_t = uint32_t;
using byte_t = std::byte;

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
    static constexpr digit_t reorder(digit_t index)
    {
        return index;
    }
};

struct BigEndian
{
    template<typename T>
    static constexpr digit_t reorder(digit_t index)
    {
        return sizeof(T) - index - 1;
    }
};

/// Universal interface for streaming bytes
class Reader
{
public:
    template<typename T>
    T read()
    {
        T result;
        this->buffer(reinterpret_cast<char*>(&result), sizeof(T));
        return result;
    }

    template<typename T, typename Endian>
    T read()
    {
        T result;
        for (digit_t i{0}; i < sizeof(T); ++i)
        {
            digit_t j = (Endian::template reorder<T>(i));
            this->read(reinterpret_cast<byte_t*>(&result) + j, 1);
        }
        return result;
    }

    template<typename T>
    void read(T* buffer, size_t count)
    {
        this->buffer(reinterpret_cast<char*>(buffer), count * sizeof(T));
    }

    virtual void skip(size_t size) = 0;
    virtual size_t tell() { return -1; }

protected:
    virtual void buffer(char* into, size_t size) = 0;
};

/// Wrap istream
template<typename Stream>
class StreamReader final : public Reader
{
public:
    template<typename... Args>
    explicit StreamReader(Args&& ...args) : stream(args...)
    {}

    void skip(size_t size) override
    {
        this->stream.seekg(size, std::ios::cur);
    }

    [[nodiscard]] size_t tell() override
    {
        return this->stream.tellg();
    }

private:
    static_assert(std::is_base_of<std::istream, Stream>::value);
    Stream stream;

    void buffer(char* into, size_t size) override
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
};

}
