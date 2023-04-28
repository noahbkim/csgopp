#pragma once

#include "../type.h"

namespace object::type
{

template<typename T, typename Base = Type>
struct WrapperType : public Base
{
    T type;

    template<typename... Args>
    WrapperType(Args&&... args) : type(std::forward<Args>(args)...) {}

    [[nodiscard]] virtual size_t size() const override { return this->type.size(); };
    [[nodiscard]] virtual size_t alignment() const override { return this->type.alignment(); };

    void construct(char* address) const override { this->type.construct(address); };
    void destroy(char* address) const override { this->type.destroy(address); };

    [[nodiscard]] virtual std::string represent() const override { return this->type.represent(); };

    virtual void emit(code::Declaration& declaration, code::Declaration::Member& member) const override
    {
        return this->type.emit(declaration, member);
    };

    virtual void emit(layout::Cursor& context) const override
    {
        return this->type.emit(context);
    }
};

}
