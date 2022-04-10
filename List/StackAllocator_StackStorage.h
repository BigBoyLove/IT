#include <memory>
#include <cstddef>

template<size_t N>
class StackStorage {
public:
    StackStorage() {}

    StackStorage(const StackStorage &) = delete;

    StackStorage &operator=(const StackStorage &) = delete;

    uint8_t *allocate(size_t bytes, size_t alignmentSize) {
        curPos += ((alignmentSize - (curPos % alignmentSize)) % alignmentSize);
        uint8_t *posToAllocate = storage + curPos;
        curPos += bytes;
        if (curPos > N) {
            curPos -= bytes;
            throw std::bad_alloc();
        }
        return posToAllocate;
    }

private:
    size_t curPos = 0;
    alignas(std::max_align_t) uint8_t storage[N]; // make on stack & Align because it faster. std::max_align_t = max shift, alignas = to align( remove shift )
};

template<typename T, size_t N>
class StackAllocator {
public:
    template<typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    StackAllocator() = delete;

    StackAllocator(StackStorage<N> &storage) : stackStorage(&storage) {}

    template<typename NewType>
    StackAllocator(const StackAllocator<NewType, N> &stackAllocator) {
        stackStorage = (stackAllocator.GetStorage());
    }

    T *allocate(size_t size) {
        return reinterpret_cast<T *>(stackStorage->allocate(size * sizeof(T), alignof(T)));
    }

    StackStorage<N> *GetStorage() const {
        return stackStorage;
    }

/// deallocate = nothing ( for speed )
    void deallocate(T *, size_t) {}

    ~StackAllocator() = default;

    using value_type = T;

private:
    StackStorage<N> *stackStorage;

};

template<typename T, typename U, size_t N>
bool operator==(const StackAllocator<T, N> &alloc1, const StackAllocator<U, N> &alloc2) {
    return alloc1.GetStorage() == alloc2.GetStorage();
}

template<typename T, typename U, size_t N>
bool operator!=(const StackAllocator<T, N> &alloc1, const StackAllocator<U, N> &alloc2) {
    return !(alloc1 == alloc2);
}