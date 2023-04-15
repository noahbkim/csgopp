#pragma once

#include <string>
#include <memory>
#include "../type.h"

namespace object::type
{

struct ArrayType : public Type
{
    std::shared_ptr<const Type> element;
    size_t length{0};

    ArrayType() = default;
    ArrayType(std::shared_ptr<const Type> element, size_t length);

    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    [[nodiscard]] std::string represent() const override;

    void emit(code::Declaration& declaration, code::Declaration::Member& member) const override;
    void emit(layout::Cursor& cursor) const override;

    [[nodiscard]] size_t at(size_t index) const;

private:
    size_t _size{0};
};

}