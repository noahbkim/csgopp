#pragma once

#include <string>
#include <memory>
#include <absl/container/flat_hash_map.h>
#include "../type.h"

namespace object::type
{

struct ObjectType : public Type
{
    struct Member
    {
        std::string name;
        std::shared_ptr<const Type> type;
        size_t offset;

        Member(const std::string& name, std::shared_ptr<const Type> type, size_t offset)
            : name(name)
            , type(std::move(type))
            , offset(offset)
        {}
    };

    using Members = std::vector<Member>;
    using MemberLookup = absl::flat_hash_map<std::string, size_t>;

    struct Builder
    {
        std::string name;
        std::shared_ptr<ObjectType> base;
        Members members;
        MemberLookup lookup;

        Builder() = default;
        explicit Builder(std::shared_ptr<ObjectType> base)
            : base(base)
            , members(base->members)
            , lookup(base->lookup)
            , _size(base->_size)
        {}

        size_t embed(const ObjectType& type);
        size_t member(const Member& member);
        size_t member(const std::string& name, std::shared_ptr<const Type> type);

        size_t size() const
        {
            return this->_size;
        }

        template<typename T = ObjectType>
        std::shared_ptr<T> build()
        {
            return std::make_shared<T>(std::move(*this));
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
    explicit ObjectType(const Builder& base);

    [[nodiscard]] size_t size() const override { return this->_size; }
    [[nodiscard]] size_t alignment() const override
    {
        return this->members.empty() ? 0 : this->members.front().type->alignment();
    }

    void construct(char* address) const override;
    void destroy(char* address) const override;

    [[nodiscard]] std::string represent() const override { return this->name; }

    [[nodiscard]] const Member& at(const std::string& name) const;

private:
    size_t _size{0};
};

}
