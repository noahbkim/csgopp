#include <memory>

namespace csgopp::common::owned
{

template<typename T>
struct owned_ptr
{
    owned_ptr()
        : pointer(nullptr), owned(false)
    {}

    explicit owned_ptr(T *pointer, bool owned = false)
        : pointer(pointer), owned(owned)
    {}

    owned_ptr(const owned_ptr<T> &other)
        : pointer(other.pointer), owned(false)
    {}

    owned_ptr(owned_ptr<T> &&other) noexcept
        : pointer(other.pointer), owned(other.owned)
    {
        other.owned = false;
    }

    owned_ptr &operator=(const owned_ptr<T> &other)
    {
        if (this != &other)
        {
            this->free();
            this->pointer = other.pointer;
            this->owned = false;
        }
        return *this;
    }

    owned_ptr &operator=(owned_ptr<T> &&other) noexcept
    {
        if (this != &other)
        {
            this->free();
            this->pointer = other.pointer;
            this->owned = other.owned;
        }
        return *this;
    }

    bool operator==(const owned_ptr& other) const { return this->pointer == other->pointer; }
    bool operator==(T* other) const { return this->pointer == other; }
    bool operator!=(const owned_ptr& other) const { return this->pointer != other->pointer; }
    bool operator!=(T* other) const { return this->pointer != other; }

    T* operator->()
    {
        return this->pointer;
    }

    const T* operator->() const
    {
        return this->pointer;
    }

    T& operator*()
    {
        return *this->pointer;
    }

    const T& operator*() const
    {
        return *this->pointer;
    }

    [[nodiscard]] T *get()
    { return this->pointer; }
    [[nodiscard]] const T *get() const
    { return this->pointer; }

private:
    T *pointer;
    bool owned;

    void free()
    {
        if (this->owned)
        {
            delete this->pointer;
        }
    }
};

template<typename T, typename... Args>
owned_ptr<T> make_owned(Args&&... args)
{
    return owned_ptr<T>(new T(std::forward<Args>(args)...));
}

}

namespace std
{
    template<typename T, typename U>
    csgopp::common::owned::owned_ptr<T> dynamic_pointer_cast(const csgopp::common::owned::owned_ptr<U>& owned)
    {
        return csgopp::common::owned::owned_ptr<T*>(dynamic_cast<T*>(owned.get()), false);
    }
}