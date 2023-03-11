#pragma once

#include <memory>
#include "error.h"
#include "magic.h"
#include "type.h"
#include "view.h"

namespace object
{

template<typename T>
struct Instance;

template<typename T>
struct BaseReference;
using Reference = BaseReference<char[]>;
using ConstantReference = BaseReference<const char[]>;

struct Lens : public View
{
    std::shared_ptr<const Type> origin;

    Lens() = default;
    Lens(std::shared_ptr<const Type> origin) : View(origin), origin(std::move(origin)) {}
    Lens(std::shared_ptr<const Type> origin, View&& view) : View(std::move(view)), origin(origin) {}
    Lens(std::shared_ptr<const Type> origin, std::shared_ptr<const Type> type, size_t offset = 0)
        : View(std::move(type), offset)
        , origin(std::move(origin))
    {}

    Lens operator[](const std::string& name) const { return Lens(this->origin, std::move(View::operator[](name))); }
    Lens operator[](size_t index) const { return Lens(this->origin, std::move(View::operator[](index))); }

    template<typename T>
    Reference operator()(std::shared_ptr<Instance<T>> instance);

    template<typename T>
    ConstantReference operator()(std::shared_ptr<const Instance<T>> instance);

    bool operator==(const Lens& l) const { return this->origin == l.origin && View::operator==(l); }
    bool operator!=(const Lens& l) const { return this->origin == l.origin && View::operator!=(l); }
    bool operator>(const Lens& l) const { return this->origin == l.origin && View::operator>(l); }
    bool operator>=(const Lens& l) const { return this->origin == l.origin && View::operator>=(l); }
    bool operator<(const Lens& l) const { return this->origin == l.origin && View::operator<(l); }
    bool operator<=(const Lens& l) const { return this->origin == l.origin && View::operator<=(l); }
};

}
