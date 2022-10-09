#pragma once

#include <vector>
#include <cstdint>
#include <google/protobuf/io/coded_stream.h>

#include "./macro.h"

namespace csgopp::common::bits
{

using google::protobuf::io::CodedInputStream;

constexpr uint8_t left(uint8_t n)
{
    return 0xFF << n;
}

constexpr uint8_t right(uint8_t n)
{
    return 0xFF >> n;
}

template<typename T, typename S = uint8_t>
constexpr S width(T t)
{
    S width{0};
    while (t > 0)
    {
        t >>= 1;
        width += 1;
    }

    // 1 -> 0, width = 1, zero bits needed
    if (width > 0)
    {
        width -= 1;
    }

    return width;
}

class BitStream
{
public:
    explicit BitStream(CodedInputStream& stream, size_t size) : data(size)
    {
        OK(stream.ReadRaw(this->data.data(), size));
    }

    explicit BitStream(const std::string& string) : data(string.size())
    {
        memcpy(this->data.data(), string.data(), string.size());
    }

    explicit BitStream(const std::vector<uint8_t>& vector) : data(vector.size())
    {
        memcpy(this->data.data(), vector.data(), vector.size());
    }

    bool skip(size_t bits)
    {
        size_t from_byte_index = bits + this->bit_index;
        this->byte_index += from_byte_index / 8;
        this->bit_index = from_byte_index % 8;
        return this->byte_index < this->data.size();
    }

    void reset()
    {
        this->byte_index = 0;
        this->bit_index = 0;
    }

    template<typename T>
    bool read(T* value, size_t bits)
    {
        // Reading up until the absolute last bit is fine, so we need a ceil
        size_t stop_byte_index = this->byte_index + (this->bit_index + bits + 7) / 8;
        if (stop_byte_index > this->data.size())
        {
            return false;
        }

        *value = 0;

        // Short case: smaller than remaining window
        if (8 - this->bit_index > bits)
        {
            *value |= (this->data.at(this->byte_index) & right(8 - bits - this->bit_index)) >> this->bit_index;
            this->bit_index += bits;
            return true;
        }

        // Take window
        *value |= this->data.at(this->byte_index) >> this->bit_index;
        size_t cursor = 8 - this->bit_index;
        this->bit_index = 0;
        this->byte_index += 1;

        while (bits - cursor > 8)
        {
            *value |= static_cast<T>(this->data.at(this->byte_index)) << cursor;
            this->byte_index += 1;
            cursor += 8;
        }

        if (bits - cursor > 0)
        {
            *value |= static_cast<T>(this->data.at(this->byte_index) & right(8 - (bits - cursor))) << cursor;
            this->bit_index = bits - cursor;
        }

        return true;
    }

private:
    std::vector<uint8_t> data;
    size_t byte_index{};
    uint8_t bit_index{};
};

}
