#pragma once

#include <vector>

namespace csgopp::common::database
{

template<typename T>
struct Noop
{
    static void destroy(T*) {}
};

template<typename T>
struct Delete
{
    static void destroy(T* item)
    {
        delete item;
    }
};

template<typename T, typename Manager = Noop<T>>
struct Database
{
    using Vector = std::vector<T*>;

    Vector container;

    Database() = default;

    explicit Database(size_t reserved)
    {
        container.reserve(reserved);
    }

    ~Database()
    {
        for (T* item : this->container)
        {
            Manager::destroy(item);
        }
    }

    [[nodiscard]] T*& at(size_t index)
    {
        return this->container.at(index);
    }

    [[nodiscard]] const T* at(size_t index) const
    {
        return this->container.at(index);
    }

    [[nodiscard]] T*& back()
    {
        return this->container.back();
    }

    [[nodiscard]] const T* back() const
    {
        return this->container.back();
    }

    [[nodiscard]] size_t size() const
    {
        return this->container.size();
    }

    typename std::vector<T*>::iterator begin()
    {
        return this->container.begin();
    }

    typename std::vector<T*>::iterator end()
    {
        return this->container.end();
    }

    typename std::vector<T*>::const_iterator begin() const
    {
        return this->container.cbegin();
    }

    typename std::vector<T*>::const_iterator end() const
    {
        return this->container.cend();
    }

    virtual void emplace(T* item)
    {
        this->container.emplace_back(item);
    }

    virtual void reserve(size_t count)
    {
        this->container.reserve(count);
    }
};

template<typename T>
struct NameTableMixin
{
    using NameTable = absl::flat_hash_map<std::string_view, T*>;
    NameTable by_name;

    NameTableMixin() = default;
    explicit NameTableMixin(size_t reserved) : by_name(reserved) {}

    [[nodiscard]] T*& at(std::string_view name)
    {
        return this->by_name.at(name);
    }

    [[nodiscard]] bool contains(std::string_view name)
    {
        return this->by_name.contains(name);
    }

    [[nodiscard]] const T* at(std::string_view name) const
    {
        return this->by_name.at(name);
    }

    void emplace(T* member)
    {
        this->by_name.emplace(member->name, member);
    }

    void reserve(size_t count)
    {
        this->by_name.reserve(count);
    }
};

template<typename T, typename Manager = Noop<T>>
struct DatabaseWithName : public Database<T, Manager>, public NameTableMixin<T>
{
    DatabaseWithName() = default;
    explicit DatabaseWithName(size_t reserved)
        : Database<T, Manager>(reserved)
        , NameTableMixin<T>(reserved) {}

    using Database<T, Manager>::at;
    using NameTableMixin<T>::at;

    void emplace(T* property) override
    {
        Database<T, Manager>::emplace(property);
        NameTableMixin<T>::emplace(property);
    }

    void reserve(size_t count) override
    {
        Database<T, Manager>::reserve(count);
        NameTableMixin<T>::reserve(count);
    }
};

template<typename T>
struct IdTableMixin
{
    using IdTable = absl::flat_hash_map<typename T::Id, T*>;
    IdTable by_id;

    IdTableMixin() = default;
    explicit IdTableMixin(size_t reserved) : by_id(reserved) {}

    [[nodiscard]] T*& at(typename T::Id name)
    {
        return this->by_id.at(name);
    }

    [[nodiscard]] const T* at(typename T::Id name) const
    {
        return this->by_id.at(name);
    }

    void emplace(T* member)
    {
        this->by_id.emplace(member->id, member);
    }

    void reserve(size_t count)
    {
        this->by_id.reserve(count);
    }
};

template<typename T, typename Manager = Noop<T>>
struct DatabaseWithNameId : public DatabaseWithName<T, Manager>, public IdTableMixin<T>
{
    DatabaseWithNameId() = default;
    explicit DatabaseWithNameId(size_t reserved)
        : DatabaseWithName<T, Manager>(reserved)
        , IdTableMixin<T>(reserved) {}

    using DatabaseWithName<T, Manager>::at;
    using IdTableMixin<T>::at;

    void emplace(T* property) override
    {
        DatabaseWithName<T, Manager>::emplace(property);
        IdTableMixin<T>::emplace(property);
    }

    void reserve(size_t count) override
    {
        DatabaseWithName<T, Manager>::reserve(count);
        IdTableMixin<T>::reserve(count);
    }
};

}
