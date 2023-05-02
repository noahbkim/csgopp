#pragma once

#include <type_traits>
#include <typeinfo>
#include <string>
#include "../error.h"
#include "../type.h"

namespace objective::type
{

template<typename T>
struct ValueType : public Type
{
    static_assert(std::is_constructible<T>::value);
    static_assert(std::is_destructible<T>::value);

    using Value = T;

    [[nodiscard]] size_t size() const override
    {
        return sizeof(T);
    }

    [[nodiscard]] size_t alignment() const override
    {
        return std::alignment_of<T>();
    }

    void construct(char* address) const override
    {
        new(address) T;
    }

    void destroy(char* address) const override
    {
        reinterpret_cast<T*>(address)->~T();
    }

    [[nodiscard]] const std::type_info& info() const override
    {
        return typeid(T);
    }

    [[nodiscard]] virtual std::string represent() const override
    {
        return typeid(T).name();
    }

    void emit(code::Declaration& declaration, code::Declaration::Member& member) const override
    {
        member.type = this->represent();
    }

    void emit(layout::Cursor& cursor) const override
    {
        cursor.note(this->represent());
    }

    [[nodiscard]] std::vector<std::string> keys() const override
    {
        throw TypeError("TrivialValueType has no keys!");
    }

    [[nodiscard]] size_t count() const override
    {
        throw TypeError("TrivialValueType has no length!");
    }
};

}