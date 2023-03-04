#pragma once

#include <stdexcept>
#include <variant>

namespace csgopy::view
{

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class View
{
public:
    [[nodiscard]] bool is_valid() const
    {
        return std::visit(overloaded {
            [](bool owned) { return owned; },
            [](const bool* referenced) { return *referenced; }
        }, this->valid);
    }

    void invalidate()
    {
        this->valid = false;
    }

    void reference(const View* other)
    {
        this->valid = &other->valid;
    }

protected:
    void assert_valid() const
    {
        if (!this->is_valid())
        {
            throw std::exception("View is no longer valid");
        }
    }

private:
    std::variant<bool, const bool*> valid{true};
};

}