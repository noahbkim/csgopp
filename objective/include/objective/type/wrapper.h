#pragma once

#include "../type.h"

namespace objective::type
{

template<typename T, typename Base = Type>
struct WrapperType : public Base
{
    T type;

    template<typename... Args>
    explicit WrapperType(Args&&... args) : type(std::forward<Args>(args)...) {}

    [[nodiscard]] size_t size() const override { return this->type.size(); }
    [[nodiscard]] size_t alignment() const override { return this->type.alignment(); }
    [[nodiscard]] const std::type_info& info() const override { return this->type.info(); }

    void construct(char* address) const override { this->type.construct(address); }
    void destroy(char* address) const override { this->type.destroy(address); }

    [[nodiscard]] std::string represent() const override { return this->type.represent(); }
    [[nodiscard]] std::vector<std::string> keys() const override { return this->type.keys(); }
    [[nodiscard]] size_t count() const override { return this->type.count(); }

    void emit(code::Declaration& declaration, code::Declaration::Member& member) const override
    {
        return this->type.emit(declaration, member);
    };

    void emit(layout::Cursor& context) const override
    {
        return this->type.emit(context);
    }
};

}
