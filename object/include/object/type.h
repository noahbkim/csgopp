#pragma once

#include <string>

namespace object
{

static constexpr size_t align(size_t offset, size_t alignment)
{
    if (alignment == 0)
    {
        return offset;
    }
    else
    {
        return (offset + alignment - 1) / alignment * alignment;
    }
}

struct Type
{
    virtual ~Type() = default;

    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual size_t alignment() const = 0;

    virtual void construct(char* address) const = 0;
    virtual void destroy(char* address) const = 0;

    [[nodiscard]] virtual std::string represent() const = 0;
};

}
