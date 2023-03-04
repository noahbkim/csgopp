#pragma once

#include <vector>
#include <absl/container/flat_hash_map.h>

namespace csgopp::common::database
{

template<typename T>
struct Database
{
    using Value = T;
    using Vector = std::vector<std::shared_ptr<Value>>;

    Vector container;

    Database() = default;

    explicit Database(size_t reserved)
    {
        container.reserve(reserved);
    }

    [[nodiscard]] std::shared_ptr<Value>& front()
    {
        return this->container.front();
    }

    [[nodiscard]] const std::shared_ptr<Value>& front() const
    {
        return this->container.front();
    }

    [[nodiscard]] std::shared_ptr<Value>& at(size_t index)
    {
        return this->container.at(index);
    }

    [[nodiscard]] const std::shared_ptr<Value>& at(size_t index) const
    {
        return this->container.at(index);
    }

    [[nodiscard]] std::shared_ptr<Value>& back()
    {
        return this->container.back();
    }

    [[nodiscard]] const std::shared_ptr<Value>& back() const
    {
        return this->container.back();
    }

    [[nodiscard]] std::shared_ptr<Value> get(size_t index)
    {
        return index < this->container.size() ? this->container.at(index) : nullptr;
    }

    [[nodiscard]] std::shared_ptr<Value> get(size_t index) const
    {
        return index < this->container.size() ? this->container.at(index) : nullptr;
    }

    [[nodiscard]] size_t size() const
    {
        return this->container.size();
    }

    typename Vector::iterator begin()
    {
        return this->container.begin();
    }

    typename Vector::iterator end()
    {
        return this->container.end();
    }

    typename Vector::const_iterator begin() const
    {
        return this->container.cbegin();
    }

    typename Vector::const_iterator end() const
    {
        return this->container.cend();
    }

    virtual void emplace(std::shared_ptr<Value> item)
    {
        this->container.emplace_back(std::move(item));
    }

    virtual void emplace(size_t index, std::shared_ptr<Value> item)
    {
        if (index >= this->container.size())
        {
            this->container.resize(index + 1);
        }
        this->container.at(index) = std::move(item);
    }

    virtual void reserve(size_t count)
    {
        this->container.reserve(count);
    }
};

template<typename T>
struct NameTableMixin
{
    using NameTable = absl::flat_hash_map<std::string_view, std::shared_ptr<T>>;
    NameTable by_name{};

    NameTableMixin() = default;

    explicit NameTableMixin(size_t reserved) : by_name(reserved)
    {
    }

    [[nodiscard]] std::shared_ptr<T>& at_name(std::string_view name)
    {
        return this->by_name.at(name);
    }

    [[nodiscard]] const std::shared_ptr<T>& at_name(std::string_view name) const
    {
        return this->by_name.at(name);
    }

    [[nodiscard]] bool contains(std::string_view name)
    {
        return this->by_name.contains(name);
    }

    void emplace(std::shared_ptr<T> member)
    {
        this->by_name.emplace(member->name, std::move(member));
    }

    void reserve(size_t count)
    {
        this->by_name.reserve(count);
    }
};

template<typename T>
struct DatabaseWithName : public Database<T>, public NameTableMixin<T>
{
    DatabaseWithName() = default;

    explicit DatabaseWithName(size_t reserved)
        : Database<T>(reserved), NameTableMixin<T>(reserved)
    {
    }

    void emplace(std::shared_ptr<T> property) override
    {
        NameTableMixin<T>::emplace(property);
        Database<T>::emplace(std::move(property));
    }

    void emplace(size_t index, std::shared_ptr<T> property) override
    {
        NameTableMixin<T>::emplace(property);
        Database<T>::emplace(index, std::move(property));
    }

    void reserve(size_t count) override
    {
        Database<T>::reserve(count);
        NameTableMixin<T>::reserve(count);
    }
};

template<typename T>
struct IdTableMixin
{
    using IdTable = absl::flat_hash_map<typename T::Id, std::shared_ptr<T>>;
    IdTable by_id{};

    IdTableMixin() = default;

    explicit IdTableMixin(size_t reserved) : by_id(reserved)
    {
    }

    [[nodiscard]] std::shared_ptr<T>& at_id(typename T::Id id)
    {
        return this->by_id.at(id);
    }

    [[nodiscard]] const std::shared_ptr<T>& at_id(typename T::Id id) const
    {
        return this->by_id.at(id);
    }

    [[nodiscard]] bool contains(typename T::Id id)
    {
        return this->by_name.contains(id);
    }

    void emplace(std::shared_ptr<T> member)
    {
        this->by_id.emplace(member->id, std::move(member));
    }

    void reserve(size_t count)
    {
        this->by_id.reserve(count);
    }
};


template<typename T>
struct DatabaseWithId : public Database<T>, public IdTableMixin<T>
{
    DatabaseWithId() = default;

    explicit DatabaseWithId(size_t reserved)
        : Database<T>(reserved)
        , IdTableMixin<T>(reserved)
    {
    }

    void emplace(std::shared_ptr<T> property) override
    {
        IdTableMixin<T>::emplace(property);
        Database<T>::emplace(std::move(property));
    }

    void emplace(size_t index, std::shared_ptr<T> property) override
    {
        IdTableMixin<T>::emplace(property);
        Database<T>::emplace(index, std::move(property));
    }

    void reserve(size_t count) override
    {
        Database<T>::reserve(count);
        IdTableMixin<T>::reserve(count);
    }
};

template<typename T>
struct DatabaseWithNameId : public DatabaseWithName<T>, public IdTableMixin<T>
{
    DatabaseWithNameId() = default;

    explicit DatabaseWithNameId(size_t reserved)
        : DatabaseWithName<T>(reserved)
        , IdTableMixin<T>(reserved)
    {
    }

    void emplace(std::shared_ptr<T> property) override
    {
        IdTableMixin<T>::emplace(property);
        DatabaseWithName<T>::emplace(std::move(property));
    }

    void reserve(size_t count) override
    {
        DatabaseWithName<T>::reserve(count);
        IdTableMixin<T>::reserve(count);
    }
};

}
