#pragma once

#include <vector>

namespace csgopp::common::database
{

template<typename T>
struct Noop
{
    static void destroy(T *)
    {}
};

template<typename T>
struct Delete
{
    static void destroy(T *item)
    {
        delete item;
    }
};

template<typename T, typename Manager = Noop<T>>
struct Database
{
    using Vector = std::vector<T *>;
    using Value = T;

    Vector container;

    Database() = default;

    explicit Database(size_t reserved)
    {
        container.reserve(reserved);
    }

    ~Database()
    {
        for (T *item: this->container)
        {
            Manager::destroy(item);
        }
    }

    [[nodiscard]] Value *&front() { return this->container.front(); }
    [[nodiscard]] const Value *front() const { return this->container.front(); }
    [[nodiscard]] Value *&at(size_t index) { return this->container.at(index); }
    [[nodiscard]] const Value *at(size_t index) const { return this->container.at(index); }
    [[nodiscard]] Value *&back() { return this->container.back(); }
    [[nodiscard]] const Value *back() const { return this->container.back(); }

    [[nodiscard]] size_t size() const { return this->container.size(); }

    typename Vector::iterator begin() { return this->container.begin(); }
    typename Vector::iterator end() { return this->container.end(); }
    typename Vector::const_iterator begin() const { return this->container.cbegin(); }
    typename Vector::const_iterator end() const { return this->container.cend(); }

    virtual void emplace(T *item) { this->container.emplace_back(item); }
    virtual void emplace(size_t index, T* item)
    {
        if (this->container.size() <= index)
        {
            this->container.resize(index + 1);
        }
        else if (this->container.at(index) != nullptr)
        {
            delete this->container.at(index);
        }
        
        this->container.at(index) = item;
    }

    virtual void reserve(size_t count)
    {
        this->container.reserve(count);
    }
};

template<typename T>
struct NameTableMixin
{
    using NameTable = absl::flat_hash_map<std::string, T *>;
    NameTable by_name{};

    NameTableMixin() = default;
    explicit NameTableMixin(size_t reserved) : by_name(reserved)
    {}

    [[nodiscard]] T *&at(std::string_view name)
    {
        return this->by_name.at(name);
    }

    [[nodiscard]] const T *at(std::string_view name) const
    {
        return this->by_name.at(name);
    }

    [[nodiscard]] bool contains(std::string_view name)
    {
        return this->by_name.contains(name);
    }

    void emplace(T *member)
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
        : Database<T, Manager>(reserved), NameTableMixin<T>(reserved)
    {}

    using Database<T, Manager>::at;
    using NameTableMixin<T>::at;

    void emplace(T *property) override
    {
        Database<T, Manager>::emplace(property);
        NameTableMixin<T>::emplace(property);
    }

    void emplace(size_t index, T *property) override
    {
        Database<T, Manager>::emplace(index, property);
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
    using IdTable = absl::flat_hash_map<typename T::Id, T *>;
    IdTable by_id{};

    IdTableMixin() = default;
    explicit IdTableMixin(size_t reserved) : by_id(reserved)
    {}

    [[nodiscard]] T *&at(typename T::Id id)
    {
        return this->by_id.at(id);
    }

    [[nodiscard]] const T *at(typename T::Id id) const
    {
        return this->by_id.at(id);
    }

    [[nodiscard]] bool contains(typename T::Id id)
    {
        return this->by_name.contains(id);
    }

    void emplace(T *member)
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
        : DatabaseWithName<T, Manager>(reserved), IdTableMixin<T>(reserved)
    {}

    using DatabaseWithName<T, Manager>::at;
    using IdTableMixin<T>::at;

    void emplace(T *property) override
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
