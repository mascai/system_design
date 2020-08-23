#include <iostream> // cout
#include <type_traits> // is_convertible
#include <algorithm> // swap
#include <cassert> // assert


template<typename T>
class uniquePtr {
    T* mPtr;
public:
    uniquePtr(const uniquePtr* other) = delete;
    uniquePtr& operator=(uniquePtr& other) = delete;
public:
    explicit uniquePtr(T* ptr = nullptr) noexcept
        : mPtr(ptr) {}

    uniquePtr(uniquePtr&& other) noexcept
        : mPtr(other.release()) {}

    template<class U>
    uniquePtr(uniquePtr<U>&& other) noexcept
        : mPtr(other.release()) {
        static_assert(std::is_convertible<T*, U*>::value, "Conversion failed");
    }

    uniquePtr& operator=(uniquePtr&& other) noexcept {
        reset(other.release());
        return *this;
    }

    template<class U>
    uniquePtr& operator=(uniquePtr&& other) noexcept {
        static_assert(std::is_convertible<T*, U>::value, "Conversion failed");
        reset(other.release());
        return *this;
    }

    void swap(uniquePtr&& other) {
        std::swap(other.mPtr, mPtr);
    }

    ~uniquePtr() {
        delete mPtr;
    }

    T* release() {
        T* temp = mPtr;
        mPtr = nullptr;
        return temp;
    }

    void reset(T* ptr = nullptr) noexcept {
        if (mPtr != ptr) {
            delete mPtr;
            mPtr = ptr;
        }
    }

    T* get() const noexcept {
        return mPtr;
    }

    T& operator*() const noexcept {
        assert(mPtr && "Invalid pointer");
        return *mPtr;
    }

    T* operator->() const noexcept {
        assert(mPtr && "Invalid pointer");
        return mPtr;
    }
};


int main() {
    uniquePtr<int> first { new int(5) };
    std::cout << *first << std::endl;
    uniquePtr<int> second { std::move(first) };
    std::cout << *second << std::endl;

    return 0;
}
