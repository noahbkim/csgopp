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

struct Type
{
    virtual ~Type() {}

    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual size_t alignment() const = 0;
    [[nodiscard]] virtual std::string represent() const = 0;
    virtual void construct(char* address) const = 0;
    virtual void destroy(char* address) const = 0;
};

struct ValueType : public Type
{
    [[nodiscard]] virtual const std::type_info& info() const = 0;
};

struct ArrayType : public Type
{
    std::shared_ptr<const Type> element;
    size_t length{0};

    ArrayType() = default;
    ArrayType(std::shared_ptr<const Type> element, size_t length);

    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;
    [[nodiscard]] std::string represent() const override;
    void construct(char* address) const override;
    void destroy(char* address) const override;

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
    using MemberLookup = absl::flat_hash_map<std::string, size_t>;

    std::string name;
    std::shared_ptr<struct ObjectType> base;
    Members members;
    MemberLookup member_lookup;

    [[nodiscard]] const Member& at(std::string_view name) const;
};


}
