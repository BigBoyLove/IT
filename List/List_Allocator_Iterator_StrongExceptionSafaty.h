#include <utility>
#include <iostream>
//#include "stackallocator.cpp"



//void *operator new(size_t n) {
//    std::cout << "new " << n << '\n';
//    return malloc(n);
//}
//
//void operator delete(void *ptr, size_t n) {
//    std::cout << "Deallocated " << n << " bytes\n";
//    free(ptr);
//}

template<typename T, typename Alloc = std::allocator<T>>
class List {
private:
    struct Node {
        Node() noexcept = default;

        Node(const Node &t) noexcept = default;

        Node &operator=(const Node &t) noexcept = default;

        Node(Node *prev, Node *next) noexcept: next(next), prev(prev), data(nullptr) {}

        Node(Node *prev, Node *next, T *val) noexcept: next(next), prev(prev), data(val) {}

        Node *next = this;
        Node *prev = this;
        T *data = nullptr;
    };

    template<typename IsConst>
    class baseIterator;

    Alloc alloc;
    using AllocTraitsT = std::allocator_traits<Alloc>;
    typename AllocTraitsT::template rebind_alloc<Node> allocNode;
    using AllocTraitsNode = std::allocator_traits<typename std::allocator_traits<Alloc>::template rebind_alloc<Node>>;
    size_t sz;
    Node *fakeNode;

public:
    using value_type = T;
    using iterator = baseIterator<T>;
    using const_iterator = baseIterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List(size_t count) : List() {
        for (size_t i = 0; i < count; ++i) {
            emplace_back();
        }
    }

    List(const Alloc &al) : alloc(al), allocNode(alloc), sz(0), fakeNode(nullptr) {
        CreateFakeNode();
    }

    List(size_t count, const T &value) : List() {
        for (size_t i = 0; i < count; ++i) {
            emplace_back(value);
        }
    }

    List(size_t count, const Alloc &al) : List(al) {
        for (size_t i = 0; i < count; ++i) {
            emplace_back();
        }
    }

    List(size_t count, const T &value, const Alloc &al) : List(al) {
        for (size_t i = 0; i < count; ++i) {
            emplace_back(value);
        }
    }

    template<typename = std::enable_if<std::is_default_constructible_v<Alloc>>>
    List() : List(Alloc()) {}

    ~List() noexcept {
        clear();
        AllocTraitsNode::deallocate(allocNode, static_cast<Node *>(fakeNode), sz + 1);
        sz = 0;
    }

//    List(const List<T, Alloc> &other) : alloc(AllocTraitsT::select_on_container_copy_construction(other.alloc)),
//                                        allocNode(AllocTraitsNode::select_on_container_copy_construction(
//                                                other.allocNode)), fakeNode(nullptr), sz(0) {
//        CreateFakeNode();
//        for (auto it = other.begin(); it != other.end(); ++it) {
//            emplace_back(*it);
//        }
//    }
    List(const List<T, Alloc> &other) : List(AllocTraitsT::select_on_container_copy_construction(other.alloc)) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            emplace_back(*it);
        }
    }

    List &operator=(const List<T, Alloc> &other) {
        clear();
        AllocTraitsNode::deallocate(allocNode, static_cast<Node *>(fakeNode), sz + 1);
        sz = 0;
        if (AllocTraitsT::propagate_on_container_copy_assignment::value && alloc != other.alloc) {
            allocNode = other.allocNode;
            alloc = other.alloc;
        }
        CreateFakeNode();
        for (auto it = other.begin(); it != other.end(); ++it) {
            emplace_back(*it);
        }
//        if (this != &other) {
////            sz = other.sz;
////            fakeNode = other.fakeNode;
//            List<T, Alloc> copy(other);
//            std::swap(sz, copy.sz);
//            std::swap(alloc, copy.alloc);
//            std::swap(allocNode, copy.allocNode);
//            std::swap(fakeNode, copy.fakeNode);
//        }
        return *this;
    }

    size_t size() const noexcept {
        return sz;
    }

    bool empty() const noexcept {
        return sz == 0;
    }

    void clear() noexcept {
        size_t count = sz;
        for (size_t i = 0; i < count; ++i) {
            EraseNoExcept(begin());
        }
        AllocTraitsNode::destroy(allocNode, fakeNode);
    }

    const Alloc &get_allocator() const noexcept { return alloc; }

    Alloc &get_allocator() noexcept { return alloc; }

    T &front() {
        if (sz == 0) throw std::out_of_range("Mimo");
        return static_cast<Node *>(fakeNode->next)->data;
    }

    const T &front() const {
        if (sz == 0) throw std::out_of_range("Mimo");
        return static_cast<Node *>(fakeNode->next)->data;
    }

    T &back() {
        if (sz == 0) throw std::out_of_range("Mimo");
        return static_cast<Node *>(fakeNode->prev)->data;
    }

    const T &back() const {
        if (sz == 0) throw std::out_of_range("Mimo");
        return static_cast<Node *>(fakeNode->prev)->data;
    }

    iterator begin() noexcept { return iterator(fakeNode->next); }

    const_iterator begin() const noexcept { return cbegin(); }

    iterator end() noexcept { return iterator(fakeNode); }

    const_iterator end() const noexcept { return cend(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    const_reverse_iterator rend() const noexcept { return crend(); }

    const_iterator cbegin() const noexcept { return const_iterator(fakeNode->next); }

    const_iterator cend() const noexcept { return const_iterator(fakeNode); }

    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    void push_back(const T &value) {
        emplace_insert(end(), value);
    }

    template<typename... Args>
    void emplace_back(Args &...args) {
        emplace_insert(end(), args...);
    }

    void push_front(const T &value) {
        emplace_insert(begin(), value);
    }

    void pop_back() {
        erase(end() - 1);
    }

    void pop_front() {
        erase(begin());
    }

    template<typename IsConst>
    void insert(baseIterator<IsConst> pos, const T &value) {
        emplace_insert(pos, value);
    }

    template<typename IsConst>
    void erase(baseIterator<IsConst> pos) {
        if (pos.ptr == nullptr) throw std::out_of_range("Mimo");
        EraseNoExcept(pos);
    }

private:
    void CreateFakeNode() {
        try {
            fakeNode = allocNode.allocate(1);
        } catch (...) {
            throw;
        }
        try {
            AllocTraitsNode::construct(allocNode, fakeNode, Node(fakeNode, fakeNode, nullptr));
        } catch (...) {
            allocNode.deallocate(fakeNode, 1);
            throw;
        }
    }

    template<typename IsConst>
    void EraseNoExcept(baseIterator<IsConst> pos) noexcept {
        pos.ptr->prev->next = pos.ptr->next;
        pos.ptr->next->prev = pos.ptr->prev;
        --sz;
        AllocTraitsT::destroy(alloc, pos.ptr->data);
        alloc.deallocate(pos.ptr->data, 1);
        AllocTraitsNode::destroy(allocNode, pos.ptr);
        allocNode.deallocate(pos.ptr, 1);
    }

    template<typename IsConst, typename... Args>
    void emplace_insert(baseIterator<IsConst> pos, Args &... args) {
        if (pos.ptr == nullptr) throw std::out_of_range("Mimo");
        Node *new_node;
        T *val;
        try {
            new_node = allocNode.allocate(1);
        } catch (...) {
            throw;
        }
        try {
            val = alloc.allocate(1);
        } catch (...) {
            allocNode.deallocate(new_node, 1);
            throw;
        }
        try {
            AllocTraitsT::construct(alloc, val, args...);
        } catch (...) {
            allocNode.deallocate(new_node, 1);
            alloc.deallocate(val, 1);
            throw;
        }
        try {
            AllocTraitsNode::construct(allocNode, new_node, pos.ptr->prev, pos.ptr, val);
        } catch (...) {
            AllocTraitsT::destroy(alloc, val);
            allocNode.deallocate(new_node, 1);
            alloc.deallocate(val, 1);
            throw;
        }
        pos.ptr->prev->next = static_cast<Node *>(new_node);
        pos.ptr->prev = static_cast<Node *>(new_node);
        ++sz;
    }
    void Print() const {
        if (sz == 0)
            return;
        Node *current = static_cast<Node *>(fakeNode->next);
        for (size_t i = 0; i < sz; ++i) {
            std::cout << (*current->data) << " ";
            current = static_cast<Node *>(current->next);
        }
    }
};


template<typename T, typename Alloc>
template<typename ConstNonConstT>
class List<T, Alloc>::baseIterator : public std::iterator<std::bidirectional_iterator_tag, ConstNonConstT> {
private:
    Node *ptr;
public:
    friend class List<T, Alloc>;

    baseIterator(Node *ptr) : ptr(ptr) {}

    template<class U, typename = std::enable_if_t<std::is_same<U, T>::value && std::is_same<ConstNonConstT, const T>::value>>
    baseIterator(const baseIterator<U> &source) noexcept: ptr(source.ptr) {};

    template<class U>
    baseIterator(const baseIterator<ConstNonConstT> &source) noexcept: ptr(source.ptr) {};

    ConstNonConstT &operator*() noexcept { return *(ptr->data); }

    const ConstNonConstT &operator*() const noexcept { return *(ptr->data); }

    ConstNonConstT *operator->() noexcept { return (ptr->data); }

    const ConstNonConstT *operator->() const noexcept { return (ptr->data); }

    baseIterator<ConstNonConstT> &operator++() noexcept {
        ptr = ptr->next;
        return *this;
    }

    baseIterator<ConstNonConstT> operator++(int) {
        baseIterator<ConstNonConstT> copy(*this);
        ++(*this);
        return copy;
    }

    baseIterator<ConstNonConstT> &operator--() {
        ptr = ptr->prev;
        return *this;
    }

    baseIterator<ConstNonConstT> operator--(int) {
        baseIterator<ConstNonConstT> copy(*this);
        --(*this);
        return copy;
    }

    baseIterator<ConstNonConstT> operator-(size_t n) {
        auto copy = *this;
        for (size_t i = 0; i < n; ++i) {
            --(copy);
        }
        return copy;
    }

    baseIterator<ConstNonConstT> operator+(size_t n) {
        auto copy = *this;
        for (size_t i = 0; i < n; ++i) {
            ++(copy);
        }
        return copy;
    }

    template<typename IsConstOther>
    bool operator==(const baseIterator<IsConstOther> &other) const {
        return ptr == other.ptr;
    }

    template<typename IsConstOther>
    bool operator!=(const baseIterator<IsConstOther> &other) const {
        return !(*this == other);
    }

    friend std::conditional_t<std::is_same_v<List<T, Alloc>, const List<T, Alloc>>, baseIterator<T>, baseIterator<const T>>;

};

