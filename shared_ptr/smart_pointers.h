#include <iostream>

template<typename T, typename Deleter>
struct DefaultDeleter {
    Deleter deleter;

    DefaultDeleter(Deleter d) : deleter(d) {}

    void operator()(T *ptr) {
        std::allocator_traits<Deleter>::destroy(deleter, ptr);
    }
};

struct BaseControlBlock {
    virtual ~BaseControlBlock() {}

    virtual void operator()(void *) = 0;

    virtual void incSharedCount() = 0;

    virtual void decSharedCount() = 0;

    virtual void incWeakCount() = 0;

    virtual void decWeakCount() = 0;

    virtual size_t getSharedCount() = 0;

    virtual size_t getWeakCount() = 0;

    virtual void destroy() = 0;
};

template<typename T, typename Alloc, typename Deleter>
struct ControlBlock;

template<typename T, typename Alloc, typename Deleter>
struct ControlBlockAndObject {
    ControlBlock<T, Alloc, Deleter> controlBlock;
    T object;
};

template<typename T, typename Alloc, typename Deleter>
struct ControlBlock : public BaseControlBlock {
public:
    size_t countShared;
    size_t countWeak;
    Deleter deleter;
    Alloc alloc;

    ControlBlock(Deleter d = Deleter(), Alloc alloc = Alloc()) : countShared(1), countWeak(0),
                                                                 deleter(d), alloc(alloc) {}

    void operator()(void *ptr) override {
        deleter(reinterpret_cast<T *>(ptr));
    }

    void incSharedCount() override {
        ++countShared;
    }

    void decSharedCount() override {
        --countShared;
    }

    void incWeakCount() override {
        ++countWeak;
    }

    void decWeakCount() override {
        --countWeak;
    }

    size_t getSharedCount() override {
        return countShared;
    }

    size_t getWeakCount() override {
        return countWeak;
    }

    void destroy() override {
        using ControlBlockAndObjectType = ControlBlockAndObject<T, Alloc, Deleter>;
        using AllocData = typename std::allocator_traits<Alloc>::template rebind_alloc<std::conditional_t<std::is_same_v<Deleter, DefaultDeleter<T, Alloc>>, ControlBlockAndObjectType, ControlBlock<T, Alloc, Deleter> > >;
        AllocData allocData = alloc;
        this->~ControlBlock();
        if constexpr (std::is_same_v<Deleter, DefaultDeleter<T, Alloc>>) {
            std::allocator_traits<AllocData>::deallocate(allocData, reinterpret_cast<ControlBlockAndObjectType *>(this), 1);
        } else {
            std::allocator_traits<AllocData>::deallocate(allocData, this, 1);
        }
    }

};

template<typename T, typename U>
using isBaseOrSame = std::enable_if_t<std::is_same_v<T, U> || std::is_base_of_v<T, U>>;

template<typename T>
class SharedPtr {
public:
    SharedPtr() : data(nullptr), ptr(nullptr) {}

    template<typename U, typename Deleter, typename Alloc, typename = isBaseOrSame<T, U>>
    SharedPtr(U *ptr, Deleter d, Alloc alloc): data(
            typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlock<U, Alloc, Deleter> >(
                    alloc).allocate(1)), ptr(ptr) {
        new(data) ControlBlock<U, Alloc, Deleter>(d, alloc);
    }


    template<typename U, typename = isBaseOrSame<T, U>>
    SharedPtr(U *ptr): SharedPtr(ptr, std::default_delete<U>(), std::allocator<U>()) {}

    SharedPtr(const SharedPtr &other) : data(other.data), ptr(other.ptr) {
        if (data != nullptr)
            data->incSharedCount();
    }

    template<typename U, typename = isBaseOrSame<T, U>>
    SharedPtr(const SharedPtr<U> &other): data(other.data), ptr(other.ptr) {
        if (data != nullptr)
            data->incSharedCount();
    }

    template<typename U, typename Deleter, typename = isBaseOrSame<T, U>>
    SharedPtr(U *ptr, Deleter d): SharedPtr(ptr, d, std::allocator<U>()) {}

    template<typename U, typename = isBaseOrSame<T, U>>
    SharedPtr(SharedPtr<U> &&other): data(other.data), ptr(other.ptr) {
        other.ptr = nullptr;
        other.data = nullptr;
    }

    SharedPtr(SharedPtr &&other) : data(other.data), ptr(other.ptr) {
        other.ptr = nullptr;
        other.data = nullptr;
    }

    template<typename U>
    SharedPtr<T> &operator=(U &&other) {
        this->template swap(SharedPtr<T>(std::forward<U>(other)));
        return *this;
    }

    ~SharedPtr() {
        if (data == nullptr) return;
        data->decSharedCount();
        if (data->getSharedCount() > 0) return;
        data->operator()(reinterpret_cast<void *>(ptr));
        if (data->getWeakCount() == 0) data->destroy();
    }

    size_t use_count() const {
        return (data ? data->getSharedCount() : 0);
    }

    template<typename U, typename = isBaseOrSame<T, U>>
    void reset(U *other) {
        (other == nullptr) ? reset() : this->template swap(SharedPtr<T>(other));
    }

    void reset() {
        this->template swap(SharedPtr<T>());
    }

    template<typename U>
    void swap(U &&other) {
        std::swap(data, other.data);
        std::swap(ptr, other.ptr);
    }

    T *get() const {
        return ptr;
    }

    T &operator*() const {
        return *ptr;
    }

    T *operator->() const {
        return ptr;
    }

private:
    template<typename U>
    friend
    class SharedPtr;

    template<typename U>
    friend
    class WeakPtr;

    BaseControlBlock *data; // ControlBlock or ControlBlockAndObject
    T *ptr;

    template<typename Alloc, typename... Args>
    SharedPtr(Alloc alloc, Args &&...args) {
        typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockAndObject<T, Alloc, DefaultDeleter<T, Alloc>>> allocControlBlockAndObject = alloc;

        auto *pControlBlockAndObject = allocControlBlockAndObject.allocate(1);
        data = &(pControlBlockAndObject->controlBlock);
        ptr = &(pControlBlockAndObject->object);

        std::allocator_traits<Alloc>::construct(alloc, ptr, std::forward<Args>(args)...);
        new(data) ControlBlock<T, Alloc, DefaultDeleter<T, Alloc>>(DefaultDeleter<T, Alloc>(alloc), alloc);
    }

    SharedPtr(T *ptr, BaseControlBlock *counter) : data(counter),
                                                   ptr(ptr) {
        counter->incSharedCount();
    }

    template<typename U, typename... Args>
    friend SharedPtr<U> makeShared(Args &&... args);

    template<typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc &, Args &&...);
};


template<typename T>
class WeakPtr {
public:
    WeakPtr() : data(nullptr), ptr(nullptr) {}

    WeakPtr(const WeakPtr &other) : data(other.data), ptr(other.ptr) {
        if (data != nullptr)
            data->incWeakCount();
    }

    template<typename U, typename = isBaseOrSame<T, U>>
    WeakPtr(const WeakPtr<U> &other): data(other.data), ptr(other.ptr) {
        if (data != nullptr)
            data->incWeakCount();
    }

    template<typename U, typename = isBaseOrSame<T, U>>
    WeakPtr(WeakPtr<U> &&other): data(other.data), ptr(other.ptr) {
        other.ptr = nullptr;
        other.data = nullptr;
    }

    WeakPtr(WeakPtr &&other) : data(other.data), ptr(other.ptr) {
        other.ptr = nullptr;
        other.data = nullptr;
    }

    template<typename U, typename = isBaseOrSame<T, U>>
    WeakPtr(const SharedPtr<U> &other): data(other.data), ptr(other.ptr) {
        data->incWeakCount();
    }

    ~WeakPtr() {
        if (data == nullptr) return;
        data->decWeakCount();
        if (data->getWeakCount() == 0 && data->getSharedCount() == 0)
            data->destroy();
    }

    template<typename U, typename = isBaseOrSame<T, U>>
    WeakPtr<T> &operator=(const SharedPtr<U> &other) {
        this->swap(WeakPtr<T>(other));
        return *this;
    }

    template<typename Other>
    WeakPtr<T> &operator=(Other &&other) {
        this->swap(WeakPtr<T>(std::forward<Other>(other)));
        return *this;
    }

    size_t use_count() const {
        return (data ? data->getSharedCount() : 0);
    }

    template<class U>
    void swap(U &&other) {
        std::swap(data, other.data);
        std::swap(ptr, other.ptr);
    }

    T *get() const {
        return ptr;
    }

    T &operator*() const {
        return *ptr;
    }

    T *operator->() const {
        return ptr;
    }

    SharedPtr<T> lock() const {
        return expired() ? SharedPtr<T>() : SharedPtr<T>(ptr, data);
    }

    bool expired() const {
        return data->getSharedCount() == 0;
    }

private:
    template<typename U>
    friend class WeakPtr;

    template<typename U>
    friend
    class SharedPtr;

    BaseControlBlock *data; // ControlBlock or ControlBlockAndObject
    T *ptr;
};

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args &&... args) {
    return SharedPtr<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc &alloc, Args &&... args) {
    return SharedPtr<T>(alloc, std::forward<Args>(args)...);
}