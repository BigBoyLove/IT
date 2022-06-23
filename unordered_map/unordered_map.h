#include <functional>
#include <cstddef>
#include <vector>
#include <iterator>
#include <optional>
#include <type_traits>


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

//        Node &operator=(Node &&t) noexcept {
//            next = std::move(t.next);
//            prev = std::move(t.prev);
//            data = std::move(t.data);
//        }

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
        return *this;
    }

    List &operator=(List<T, Alloc> &&other) {
        if (this != &other) {
//            std::swap(fakeNode, other.fakeNode);
//            std::swap(sz, other.sz);

//            clear();
//            AllocTraitsNode::deallocate(allocNode, static_cast<Node *>(fakeNode), sz + 1);
//            sz = 0;
//            if (AllocTraitsT::propagate_on_container_copy_assignment::value && alloc != other.alloc) {
//                allocNode = other.allocNode;
//                alloc = other.alloc;
//            }
//            CreateFakeNode();
//            fakeNode->next = std::move(other.fakeNode->next);
//            fakeNode->prev = std::move(other.fakeNode->prev);
            fakeNode = std::move(other.fakeNode);
            try {
                other.fakeNode = allocNode.allocate(1);
            } catch (...) {
                throw;
            }
            try {
                AllocTraitsNode::construct(allocNode, other.fakeNode, Node(other.fakeNode, other.fakeNode, nullptr));
            } catch (...) {
                allocNode.deallocate(other.fakeNode, 1);
                throw;
            }
            sz = other.sz;
            other.sz = 0;
//            other.fakeNode->next->prev = fakeNode;
//            other.fakeNode->prev->next = fakeNode;
//            fakeNode->next = other.fakeNode->next;
//            fakeNode->prev = other.fakeNode->prev;
//            other.fakeNode->next = other.fakeNode;
//            other.fakeNode->prev = other.fakeNode;
//            other.fakeNode = nullptr;
        }
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

    template<class U, typename = std::enable_if_t<
            std::is_same<U, T>::value && std::is_same<ConstNonConstT, const T>::value>>
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

    bool IsEnd() {
        return ptr == nullptr || ptr->data == nullptr;
    }

    friend std::conditional_t<std::is_same_v<List<T, Alloc>, const List<T, Alloc>>, baseIterator<T>, baseIterator<const T>>;
};

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;
private:
    using listAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType *>;
    using listIterator = typename List<NodeType *, listAlloc>::iterator;
    using constListIterator = typename List<NodeType *, listAlloc>::const_iterator;
    using iterAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<typename List<NodeType *, listAlloc>::iterator>;
    using nodeAlloc = typename std::allocator_traits<Alloc>;

    constexpr static const float DEFAULT_MAX_LOAD_FACTOR = 0.8f;

    List<NodeType *, listAlloc> nodes; // idea to store all nodes in list
    std::vector<listIterator, iterAlloc> chunksStarts; // is needed in find to stop in time
    std::vector<listIterator, iterAlloc> chunksEnds;
    size_t chunks = 0;
    float maxLoadFactor = DEFAULT_MAX_LOAD_FACTOR;
    size_t prevChunkHash = 0; // is needed for connection between chunks

    Hash hash;
    Equal equal;
    Alloc alloc;

    template<typename IteratorType>
    class base_iterator;

public:
    using const_iterator = base_iterator<const NodeType>;
    using iterator = base_iterator<NodeType>;

    UnorderedMap()
            : nodes(), chunksStarts(std::vector<listIterator, iterAlloc>(1, nodes.end())),
              chunksEnds(std::vector<listIterator, iterAlloc>(1, nodes.end())), chunks(1), hash(),
              equal(), alloc() {}

    UnorderedMap(const UnorderedMap &other) :
            nodes(),
            chunksStarts(std::vector<listIterator, iterAlloc>(other.chunks, nodes.end())),
            chunksEnds(std::vector<listIterator, iterAlloc>(other.chunks, nodes.end())),
            chunks(other.chunks),
            maxLoadFactor(other.maxLoadFactor),
            prevChunkHash(other.prevChunkHash), hash(),
            equal(), alloc(
            std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value ? other.alloc : Alloc()) {
        for (auto it = other.cbegin(); it != other.cend(); ++it) {
            insert(*it);
        }
    }

    UnorderedMap &operator=(const UnorderedMap &other) {
        if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
            alloc = other.alloc;
        }
        nodes = other.nodes;
        chunks = other.chunks;
        chunksStarts = std::vector<listIterator, iterAlloc>(chunks, nodes.end());
        chunksEnds = std::vector<listIterator, iterAlloc>(chunks, nodes.end());
        for (auto node: nodes) {
            insert(node);
        }
        maxLoadFactor = other.maxLoadFactor;
        prevChunkHash = other.prevChunkHash;
        return *this;
    }

    UnorderedMap(UnorderedMap &&other) :
            nodes(std::move(other.nodes)),
            chunksStarts(std::move(other.chunksStarts)),
            chunksEnds(std::move(other.chunksEnds)),
            chunks(other.chunks),
            maxLoadFactor(other.maxLoadFactor),
            prevChunkHash(other.prevChunkHash), hash(other.hash),
            equal(other.equal), alloc(std::move(other.alloc)) {}

    UnorderedMap &operator=(UnorderedMap &&other) {
        alloc = std::move(alloc);
        nodes = std::move(other.nodes);
        chunksStarts = std::move(other.chunksStarts);
        chunksEnds = std::move(other.chunksEnds);
        chunks = other.chunks;
        maxLoadFactor = other.maxLoadFactor;
        prevChunkHash = other.prevChunkHash;
        return *this;
    }

    ~UnorderedMap() {
        for (auto it = nodes.cbegin(); it != nodes.cend(); ++it) {
            std::allocator_traits<Alloc>::destroy(alloc, (*it));
            std::allocator_traits<Alloc>::deallocate(alloc, (*it), 1);
        }
    }

    Value &operator[](const Key &key) {
        iterator itKVP = insert({key, Value()}).first;
        return itKVP->second;
    }

    Value &operator[](Key &&key) {
        iterator itKVP = insert({std::move(key), Value()}).first;
        return itKVP->second;
    }

    Value &at(const Key &key) {
        auto it = find(key);
        if (it == end()) throw std::out_of_range("Mimo");
        return it->second;
    }

    const Value &at(const Key &key) const {
        const auto it = find(key);
        if (it == cend()) throw std::out_of_range("Mimo");
        return it->second;
    }

    size_t size() const { return nodes.size(); }
    iterator begin() { return nodes.begin(); }
    const_iterator begin() const{ return nodes.cbegin();}
    const_iterator cbegin() const{return nodes.cbegin();}
    iterator end(){return nodes.end();}
    const_iterator end() const{ return nodes.cend();}
    const_iterator cend() const{return nodes.cend();}

    std::pair<iterator, bool> insert(const NodeType &node){
        NodeType *new_node = nodeAlloc::allocate(alloc, 1);
        nodeAlloc::construct(alloc, new_node, node);
        return insertHelp(new_node);
    }

    template<typename U>
    std::pair<iterator, bool> insert(U &&node){
        NodeType *new_node = nodeAlloc::allocate(alloc, 1);
        nodeAlloc::construct(alloc, new_node, std::forward<U>(node));
        return insertHelp(new_node);
    }

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last){
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(Args &&... args){
        NodeType *new_node = nodeAlloc::allocate(alloc, 1);
        nodeAlloc::construct(alloc, new_node, std::forward<Args>(args)...);
        return insertHelp(new_node);
    }

    iterator erase(const_iterator position){
        if (position == cend()) return end();
        size_t nodeHash = CurHash(position->first);
        iterator prev = find(position->first); // cast const to non-const
        iterator next = ++prev;

        bool isChunkStart = position.listIter == chunksStarts[nodeHash];
        bool isChunkEndInclusive = next.listIter == chunksEnds[nodeHash];

        if (isChunkStart && isChunkEndInclusive) { // only elem in chunk
            chunksStarts[nodeHash] = nodes.end();
            chunksEnds[nodeHash] = nodes.end();
        } else if (isChunkStart /* && !isChunkEndInclusive */) { // first elem in chunk
            chunksStarts[nodeHash] = next.listIter;
        } else if (/* !isChunkStart &&*/ isChunkEndInclusive) { // last elem in chunk
            chunksEnds[nodeHash] = prev.listIter; // works because after erasing iter will point on the begging of the chunk or end of the list
        }
        nodes.erase(position.listIter);
        return next;
    }

    const_iterator erase(const_iterator first, const_iterator last){
        for (auto it = first; it != last; it = erase(it));
        return last;
    }

    template<typename U>
    iterator find(U &&key){
        auto tryFind = findHelp<iterator, U>(key, CurHash(key));
        return tryFind ? tryFind.value() : end();
    }

    template<typename U>
    const_iterator find(U &&key) const{
        auto tryFind = findHelp<const_iterator, U>(key, CurHash(key));
        return tryFind ? tryFind.value() : cend();
    }

    void reserve(size_t count){
        if (count > nodes.size()) rehash(static_cast<int>(static_cast<float>(count) / maxLoadFactor) + 1);
    }

    size_t max_size() const{return chunksStarts.max_size();}

    float load_factor() const{return static_cast<float>(nodes.size()) / chunks;}

    float max_load_factor() const{
        return maxLoadFactor;
    }

    void max_load_factor(float new_max_load_factor){
        maxLoadFactor = new_max_load_factor;
        checkRehash();
    }

private:
    size_t CurHash(const Key &key) const{return hash(key) % chunks;}

    void checkRehash(){
        if (load_factor() > maxLoadFactor) reserve(chunks * 2 + 3);
    }

    void rehash(size_t count){
        List<NodeType *, listAlloc> all_nodes = std::move(nodes);
        nodes = List<NodeType *, listAlloc>();
        chunks = count;
        chunksStarts = std::vector<listIterator, iterAlloc>(chunks, nodes.end());
        chunksEnds = std::vector<listIterator, iterAlloc>(chunks, nodes.end());
        maxLoadFactor = DEFAULT_MAX_LOAD_FACTOR;
        prevChunkHash = 0;
        for (auto element: all_nodes) {
            insertHelp(element);
        }
    }

    std::pair<iterator, bool> insertHelp(NodeType *new_node){
        checkRehash();
        iterator foundPos = find(new_node->first);
        if (!foundPos.IsEnd()) return {foundPos, false}; // haven't found such node

        size_t nodeHash = CurHash(new_node->first);
        auto itChunkEnd = iterator(chunksEnds[nodeHash]);
        nodes.template insert(itChunkEnd.listIter, new_node); // if nodes is empty: == push_pack

        if (iterator(chunksStarts[nodeHash]).IsEnd()) { // current Chunk was empty
            chunksStarts[nodeHash] = (--nodes.end());
            if (size() > 1)
                chunksEnds[prevChunkHash] = chunksStarts[nodeHash]; // connect prevChunk ( "prevChunkHash" ) with current newChunk ( "nodeHash" )
            prevChunkHash = nodeHash;
            //                                                                            prevChunk  ...  newChunk
            //                                                                               #            -> #
            //                                                                              ...           |
            //                                                                               #            |
            //                                                                               |____________|

        }
        return {--itChunkEnd, true};
    }

    template<typename Iterator, typename U>
    std::optional<Iterator> findHelp(U &&key, size_t hash_node) const{
        auto ChunkStart = Iterator(chunksStarts[hash_node]);
        auto ChunkEnd = Iterator(chunksEnds[hash_node]);
        while (ChunkStart != ChunkEnd) {
            if (equal(ChunkStart->first, key)) return ChunkStart;
            ++ChunkStart;
        }
        return std::nullopt;
    }
};


template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename ConstNonConstT>
class UnorderedMap<Key, Value, Hash, Equal, Alloc>::base_iterator
        : public std::iterator<std::forward_iterator_tag, ConstNonConstT> {
public:
    bool operator==(const base_iterator &other) const {
        return listIter == other.listIter;
    }

    bool operator!=(const base_iterator &other) const {
        return !(*this == other);
    }

    ConstNonConstT &operator*() const {
        return *(*listIter);
    }

    ConstNonConstT *operator->() const {
        return (*listIter);
    }

    base_iterator &operator++() {
        ++listIter;
        return *this;
    }

    base_iterator operator++(int) {
        auto copy = *this;
        ++listIter;
        return copy;
    }

    base_iterator &operator--() {
        --listIter;
        return *this;
    }

    base_iterator operator--(int) {
        auto copy = *this;
        --listIter;
        return copy;
    }

    operator base_iterator<const ConstNonConstT>() {
        return base_iterator<const ConstNonConstT>(listIter);
    }

private:
    friend class UnorderedMap;

    typename std::conditional_t<std::is_const<ConstNonConstT>::value, constListIterator, listIterator> listIter;

    base_iterator(listIterator it) : listIter(it) {}

    base_iterator(constListIterator it) : listIter(it) {}

    bool IsEnd() {
        return listIter.IsEnd();
    }
};