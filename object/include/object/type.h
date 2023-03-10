#pragma once

#include <memory>
#include <string>
#include <vector>
#include <absl/container/flat_hash_map.h>

namespace object
{

struct Type;
struct ValueType;
struct ArrayType;
struct ObjectType;
struct Instance;

static constexpr size_t align(size_t offset, size_t alignment)
{
    if (alignment == 0)
    {
        return offset;
    }
    else
    {
        return (offset + alignment - 1) / alignment * alignment;
    }
}

struct Type
{
    virtual ~Type() {}

    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual size_t alignment() const = 0;

    virtual void construct(char* address) const = 0;
    virtual void destroy(char* address) const = 0;

    [[nodiscard]] virtual std::string represent() const = 0;
};

struct ValueType : public Type
{
    [[nodiscard]] virtual const std::type_info& info() const = 0;
};

template<typename T>
struct TrivialValueType : public ValueType
{
    static_assert(std::is_constructible<T>::value);
    static_assert(std::is_destructible<T>::value);

    using Value = T;

    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    [[nodiscard]] const std::type_info& info() const override;
    [[nodiscard]] virtual std::string represent() const override;
};

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

    [[nodiscard]] size_t at(size_t index) const;
};

struct ObjectType : public Type
{
    struct Member
    {
        std::string name;
        std::shared_ptr<const Type> type;
        size_t offset;
    };

    using Members = std::vector<Member>;
    using MemberLookup = absl::flat_hash_map<std::string_view, size_t>;

    struct Builder
    {
        std::string name;
        std::shared_ptr<ObjectType> base;
        Members members;
        MemberLookup lookup;

        Builder() = default;
        explicit Builder(auto&& base);

        size_t embed(const ObjectType& type);
        size_t member(const Member& member);
        size_t member(std::string name, std::shared_ptr<const Type> type);

        size_t size() const
        {
            return this->_size;
        }

    private:
        size_t _size{0};
    };

    std::string name;
    MemberLookup lookup;
    Members members;
    std::shared_ptr<ObjectType> base;

    ObjectType() = default;
    explicit ObjectType(Builder&& builder);

    [[nodiscard]] size_t size() const override
    {
        return this->_size;
    }

    [[nodiscard]] size_t alignment() const override
    {
        return this->members.empty() ? 0 : this->members.front().type->alignment();
    }

    void construct(char* address) const override;
    void destroy(char* address) const override;

    std::string represent() const override
    {
        return this->name;
    }

    [[nodiscard]] const Member& at(std::string_view name) const;

private:
    size_t _size{0};
};


}
