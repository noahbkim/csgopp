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
        std::weak_ptr<code::Metadata<code::Declaration&, code::Declaration::Member&>> metadata;

        Member(const std::string& name, std::shared_ptr<const Type> type, size_t offset)
            : name(name)
            , type(std::move(type))
            , offset(offset)
        {}

        void emit(code::Declaration& context, code::Declaration::Member& declaration) const;
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
        explicit Builder(std::string name) : name(name) {}
        explicit Builder(std::shared_ptr<ObjectType> base)
            : base(base)
            , members(base->members)
            , lookup(base->lookup)
            , _size(base->_size)
        {}
        explicit Builder(const std::string& name, std::shared_ptr<ObjectType> base)
            : Builder(std::move(base))
        {
            this->name = name;
        }

        size_t embed(const ObjectType& type);
        size_t member(const Member& member);
        size_t member(const std::string& name, std::shared_ptr<const Type> type);

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
    std::weak_ptr<code::Metadata<code::Declaration&>> metadata;

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

    virtual void emit(code::Declaration& declaration) const;
    void emit(code::Declaration& declaration, code::Declaration::Member& member) const override;
    void emit(layout::Cursor& cursor) const override;

    [[nodiscard]] const Member& at(const std::string& name) const;

    [[nodiscard]] Members::const_iterator begin() const;
    [[nodiscard]] Members::const_iterator begin_self() const;
    [[nodiscard]] Members::const_iterator end() const;

private:
    size_t _size{0};
};

}
