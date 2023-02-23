#pragma once

#include <vector>
#include <cstdint>
#include <google/protobuf/io/coded_stream.h>

#include "./macro.h"

/// Defines tools for manipulating compressed bit streams.
///
/// @brief hello world
namespace csgopp::common::bits
{

using google::protobuf::io::CodedInputStream;

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

class BitView
{
public:
    explicit BitView(const std::string& string)
        : data(reinterpret_cast<const uint8_t*>(string.data()))
        , data_size(string.size())
    {
    }

    explicit BitView(const std::vector<uint8_t>& data)
        : data(reinterpret_cast<const uint8_t*>(data.data()))
        , data_size(data.size())
    {
    }

    BitView(const char* data, size_t data_size)
        : data(reinterpret_cast<const uint8_t*>(data))
        , data_size(data_size)
    {
    }

    void reset()
    {
        this->data_index = 0;
        this->buffer = 0;
        this->buffer_size = 0;
    }

    bool skip(size_t bits)
    {
        if (bits > this->buffer_size)
        {
            bits -= this->buffer_size;
            this->data_index += bits / 8;
            if (this->data_index >= this->data_size)
            {
                return false;
            }

            bits %= 8;
        }

        this->buffer_size -= bits;
        this->buffer >>= bits;
        return true;
    }

    template<typename T>
    bool read(T* value, size_t bits)
    {
        *value = 0;
        if (bits == 0)
        {
            return true;
        }

        size_t value_size{0};
        do
        {
            if (this->buffer_size == 0)
            {
                if (!this->fetch())
                {
                    return false;
                }
            }

            *value |= this->buffer << value_size;
            if (bits > this->buffer_size)
            {
                bits -= this->buffer_size;
                value_size += this->buffer_size;
                this->buffer_size = 0;
            }
            else
            {
                *value &= ((static_cast<size_t>(1) << (bits + value_size)) - 1);
                this->buffer >>= bits;
                this->buffer_size -= bits;
                break;
            }
        } while (bits > 0);

        return true;
    }

protected:
    bool fetch()
    {
        if (this->data_index >= this->data_size)
        {
            return false;
        }

        this->buffer = static_cast<size_t>(this->data[this->data_index]);
        this->buffer_size += 8;
        this->data_index += 1;

        if (this->data_index == this->data_size)
        {
            return true;
        }

        size_t counter{1};
        do
        {
            this->buffer |= static_cast<size_t>(this->data[this->data_index]) << (counter * 8);
            this->buffer_size += 8;
            this->data_index += 1;
            counter += 1;
        } while (this->data_index < this->data_size && counter < sizeof(this->buffer));
        return true;
    }

private:
    const uint8_t* data;
    size_t data_size;
    size_t data_index{0};
    size_t buffer{0};
    uint8_t buffer_size{0};
};

template<typename Source>
struct BitDecoder : Source
{
    using Source::Source;

    /// Read a C-style string into the given container.
    bool read_string(std::string& string)
    {
        do
        {
            string.push_back(0);
            if (!this->read(&string.back(), 8))
            {
                return false;
            }
        } while (string.back() != 0);
        string.pop_back();

        return true;
    }

    bool read_string_from(std::string& string, size_t size)
    {
        string.clear();
        if (size == 0)
        {
            return true;
        }

        while (true)
        {
            string.push_back(0);
            if (!this->read(&string.back(), 8))
            {
                return false;
            }

            size -= 1;

            if (string.back() == 0)
            {
                string.pop_back();
                break;
            }

            if (size == 0)
            {
                return true;
            }
        }

        if (size > 0)
        {
            this->skip(size * 8);
        }

        return true;
    }

    template<typename T>
    bool read_variable_unsigned_int(T* value)
    {
        *value = 0;
        constexpr size_t limit = (sizeof(T) * 8 + 6) / 7;
        size_t i = 0;
        uint8_t cursor = 0;
        do
        {
            if (!this->read(&cursor, 8))
            {
                return false;
            }

            *value |= static_cast<T>(cursor & 0x7F) << (7 * i);
            ++i;
        } while ((cursor & 0x80) && i < limit);
        return true;
    }

    template<typename T>
    bool read_variable_signed_int(T* value)
    {
        this->read_variable_unsigned_int(value);
        *value = (*value >> 1) ^ -(*value & 1);
        return true;
    }

    bool read_compressed_uint32(uint32_t* value)
    {
        if (!this->read(value, 6))
        {
            return false;
        }

        if ((*value & 0b110000) == 0)
        {
            return true;
        }

        uint32_t buffer;
        bool ok;
        switch (*value & 0b110000)
        {
        case 0b010000:ok = this->read(&buffer, 4);
            break;
        case 0b100000:ok = this->read(&buffer, 8);
            break;
        case 0b110000:ok = this->read(&buffer, 32 - 4);
            break;
        }

        *value = (*value & 0b1111) | (buffer << 4);
        return ok;
    }

    bool read_compressed_uint16(uint16_t* value)
    {
        if (!this->read(value, 7))
        {
            return false;
        }

        if ((*value & 0b1100000) == 0)
        {
            return true;
        }

        uint32_t buffer;
        bool ok;
        switch (*value & 0b1100000)
        {
        case 0b0100000:ok = this->read(&buffer, 2);
            break;
        case 0b1000000:ok = this->read(&buffer, 4);
            break;
        case 0b1100000:ok = this->read(&buffer, 7);
            break;
        }

        *value = (*value & 0b11111) | (buffer << 5);
        return ok;
    }
};

using BitStream = BitDecoder<BitView>;

}
