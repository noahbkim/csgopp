#pragma once

#include <utility>

#include "object/error.h"
#include "object/type.h"
#include "object/type/value.h"
#include "object/type/array.h"
#include "object/type/object.h"
#include "object/view.h"

namespace object
{

using type::ValueType;
using type::TrivialValueType;
using type::ArrayType;
using type::ObjectType;

template<typename T> struct Handle;
template<typename T> struct Instance;

struct Lens;
struct Reference;
struct ConstantReference;

template<typename T>
struct Handle
{
    std::shared_ptr<const T> type;

    Handle() = default;
    explicit Handle(std::shared_ptr<const T>&& type) : type(std::move(type)) {}
    explicit Handle(const std::shared_ptr<const T>& type) : type(type) {}

    template<typename... Args>
    static Handle make(Args&&... args)
    {
        auto type = std::make_shared<const T>(std::forward<Args>(args)...);
        return Handle(std::move(type));
    }

    [[nodiscard]] Lens view() const;
    [[nodiscard]] Lens operator[](const std::string& name) const;
    [[nodiscard]] Lens operator[](size_t index) const;

    [[nodiscard]] std::shared_ptr<const T> get() const { return this->type; }
    [[nodiscard]] auto operator->() { return this->type.operator->(); }
    [[nodiscard]] auto operator->() const { return this->type.operator->(); }
};

template<typename T>
struct Instance
{
    std::shared_ptr<char[]> data;
    std::shared_ptr<const T> type;

    Instance(std::shared_ptr<const T> type, std::shared_ptr<char[]> data)
        : data(std::move(data))
        , type(std::move(type))
    {
        this->type->construct(this->data.get());
    }

    virtual ~Instance()
    {
        this->type->destroy(this->data.get());
    }

    [[nodiscard]] static Instance<T> make(const std::shared_ptr<const T>& type)
    {
        return Instance<T>(type, std::make_shared<char[]>(type->size()));
    }

    [[nodiscard]] static Instance<T> make(const Handle<T>& type)
    {
        return Instance<T>(type.get(), std::make_shared<char[]>(type->size()));
    }

    [[nodiscard]] Reference view();
    [[nodiscard]] ConstantReference view() const;
    [[nodiscard]] Reference operator[](const std::string& name);
    [[nodiscard]] ConstantReference operator[](const std::string& name) const;
    [[nodiscard]] Reference operator[](size_t index);
    [[nodiscard]] ConstantReference operator[](size_t index) const;

    template<typename U>
    [[nodiscard]] U& is() { return object::is<U>(this->type.get(), this->data.get()); }
    template<typename U>
    [[nodiscard]] const U& is() const { object::is<U>(this->type.get(), this->data.get()); }

    template<typename U>
    [[nodiscard]] U* as() { return object::as<U>(this->type.get(), this->data.get()); }
    template<typename U>
    [[nodiscard]] const U* as() const { return object::as<U>(this->type.get(), this->data.get()); }

    [[nodiscard]] std::shared_ptr<char[]> get() { return this->data; }
    [[nodiscard]] std::shared_ptr<const char[]> get() const { return this->data; }
};

using Value = Instance<ValueType>;
using Array = Instance<ArrayType>;
using Object = Instance<ObjectType>;

struct Lens : public View
{
    std::shared_ptr<const Type> origin;

    Lens() = default;
    explicit Lens(std::shared_ptr<const Type> origin) : View(origin), origin(std::move(origin)) {}
    Lens(std::shared_ptr<const Type> origin, View&& view) : View(std::move(view)), origin(std::move(origin)) {}
    Lens(std::shared_ptr<const Type> origin, std::shared_ptr<const Type> type, size_t offset = 0)
        : View(std::move(type), offset)
        , origin(std::move(origin))
    {}

    [[nodiscard]] Lens operator[](const std::string& name) const
    {
        return Lens(this->origin, std::move(View::operator[](name)));
    }

    [[nodiscard]] Lens operator[](size_t index) const
    {
        return Lens(this->origin, std::move(View::operator[](index)));
    }

    template<typename T>
    [[nodiscard]] Reference operator()(Instance<T>& instance);
    template<typename T>
    [[nodiscard]] Reference operator()(std::shared_ptr<Instance<T>> instance);

    template<typename T>
    [[nodiscard]] ConstantReference operator()(const Instance<T>& instance);
    template<typename T>
    [[nodiscard]] ConstantReference operator()(std::shared_ptr<const Instance<T>> instance);

    [[nodiscard]] bool operator==(const Lens& l) const { return this->origin == l.origin && View::operator==(l); }
    [[nodiscard]] bool operator!=(const Lens& l) const { return this->origin == l.origin && View::operator!=(l); }
    [[nodiscard]] bool operator>(const Lens& l) const { return this->origin == l.origin && View::operator>(l); }
    [[nodiscard]] bool operator>=(const Lens& l) const { return this->origin == l.origin && View::operator>=(l); }
    [[nodiscard]] bool operator<(const Lens& l) const { return this->origin == l.origin && View::operator<(l); }
    [[nodiscard]] bool operator<=(const Lens& l) const { return this->origin == l.origin && View::operator<=(l); }
};

template<typename T>
struct ReferenceBase : public Lens
{
    std::shared_ptr<T> data;

    ReferenceBase() = default;
    ReferenceBase(std::shared_ptr<T> data, Lens&& lens)
        : Lens(std::move(lens))
        , data(std::move(data))
    {}
    ReferenceBase(std::shared_ptr<const Type> origin, std::shared_ptr<T> data)
        : Lens(std::move(origin))
        , data(std::move(data))
    {}
    ReferenceBase(std::shared_ptr<const Type> origin, std::shared_ptr<T> data, View&& view)
        : Lens(std::move(origin), std::move(view))
        , data(std::move(data))
    {}
    ReferenceBase(
        std::shared_ptr<const Type> origin,
        std::shared_ptr<T> data,
        std::shared_ptr<const Type> type,
        size_t offset = 0
    )
        : Lens(std::move(origin), std::move(type), offset)
        , data(std::move(data))
    {}

    [[nodiscard]] char* get() { return this->data.get() + this->offset; }
    [[nodiscard]] const char* get() const { return this->data.get() + this->offset; }

    [[nodiscard]] ReferenceBase operator[](const std::string& name) const
    {
        return ReferenceBase(this->data, std::move(Lens::operator[](name)));
    }

    [[nodiscard]] ReferenceBase operator[](size_t index) const
    {
        return ReferenceBase(this->data, std::move(Lens::operator[](index)));
    }

    template<typename U>
    [[nodiscard]] bool operator==(const ReferenceBase<U>& r) const { return this->data == r.data && View::operator==(r); }
    template<typename U>
    [[nodiscard]] bool operator!=(const ReferenceBase<U>& r) const { return this->data == r.data && View::operator!=(r); }
    template<typename U>
    [[nodiscard]] bool operator>(const ReferenceBase<U>& r) const { return this->data == r.data && View::operator>(r); }
    template<typename U>
    [[nodiscard]] bool operator>=(const ReferenceBase<U>& r) const { return this->data == r.data && View::operator>=(r); }
    template<typename U>
    [[nodiscard]] bool operator<(const ReferenceBase<U>& r) const { return this->data == r.data && View::operator<(r); }
    template<typename U>
    [[nodiscard]] bool operator<=(const ReferenceBase<U>& r) const { return this->data == r.data && View::operator<=(r); }

    template<typename U>
    [[nodiscard]] U& is() { return object::is<U>(this->type.get(), this->get()); }
    template<typename U>
    [[nodiscard]] const U& is() const { object::is<U>(this->type.get(), this->get()); }

    template<typename U>
    [[nodiscard]] U* as() { return object::as<U>(this->type.get(), this->get()); }
    template<typename U>
    [[nodiscard]] const U* as() const { return object::as<U>(this->type.get(), this->get()); }
};

struct Reference : public ReferenceBase<char[]>
{
    using ReferenceBase::ReferenceBase;

    template<typename U>
    explicit Reference(Instance<U>& instance) : ReferenceBase(instance.type, instance.data) {}
    template<typename U>
    explicit Reference(const std::shared_ptr<Instance<U>>& instance) : ReferenceBase(instance.type, instance.data) {}
};

struct ConstantReference : public ReferenceBase<const char[]>
{
    template<typename U>
    explicit ConstantReference(const Instance<U>& instance) : ReferenceBase(instance.type, instance.data) {}
    template<typename U>
    explicit ConstantReference(const std::shared_ptr<const Instance<U>>& instance)
        : ReferenceBase(instance.type, instance.data)
    {}
};

template<typename T>
Lens Handle<T>::view() const
{
    return Lens(this->type);
}

template<typename T>
Lens Handle<T>::operator[](const std::string& name) const
{
    return Lens(this->type, std::move(View::at(this->type.get(), name)));
}

template<typename T>
Lens Handle<T>::operator[](size_t index) const
{
    return Lens(this->type, std::move(View::at(this->type.get(), index)));
}

template<typename T>
Reference Instance<T>::view()
{
    return Reference(*this);
}

template<typename T>
ConstantReference Instance<T>::view() const
{
    return ConstantReference(*this);
}

template<typename T>
Reference Instance<T>::operator[](const std::string& name)
{
    return Reference(this->type, this->data, std::move(View::at(this->type.get(), name)));
}

template<typename T>
ConstantReference Instance<T>::operator[](const std::string& name) const
{
    return ConstantReference(this->type, this->data, std::move(View::at(this->type.get(), name)));
}

template<typename T>
Reference Instance<T>::operator[](size_t index)
{
    return Reference(this->type, this->data, std::move(View::at(this->type.get(), index)));
}

template<typename T>
ConstantReference Instance<T>::operator[](size_t index) const
{
    return ConstantReference(this->type, this->data, std::move(View::at(this->type.get(), index)));
}

template<typename T>
static void validate(const Instance<T>* instance, const Type* origin)
{
    if (instance->type.get() != origin)
    {
        throw TypeError(
            concatenate(
                "Cannot use lens with origin ",
                origin->represent(),
                " on instance of type ",
                instance->type->represent()
            )
        );
    }
}

template<typename T>
Reference Lens::operator()(Instance<T>& instance)
{
    validate(&instance, this->origin.get());
    return Reference(this->origin, instance.data, this->type, this->offset);
}

template<typename T>
Reference Lens::operator()(std::shared_ptr<Instance<T>> instance)
{
    return this->operator()(*instance);
}

template<typename T>
ConstantReference Lens::operator()(const Instance<T>& instance)
{
    validate(&instance, this->origin.get());
    return ConstantReference(this->origin, instance.data, this->type, this->offset);
}

template<typename T>
ConstantReference Lens::operator()(std::shared_ptr<const Instance<T>> instance)
{
    return this->operator()(*instance);
}

}
