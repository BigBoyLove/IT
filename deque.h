#include <iostream>
#include <vector>
#include <iterator>
#include <memory.h>

template<typename T, size_t chunk_size = 1 << 16>
class Deque {
private:
    template<typename T_, typename Const>
    class baseIterator;

    static T *GetChunk() {
        return reinterpret_cast<T *>(new uint8_t[chunk_size * sizeof(T)]);
    }

    static void DelChunk(T *arr) {
        delete[] reinterpret_cast<uint8_t *>(arr);
    }

    void DelBody() noexcept {
        for (iterator iter = _begin; iter != _end; ++iter) {
            iter->~T();
        }
        size_t startChunk = _begin.chunk, endChunk = startChunk + chCap;
        for (size_t i = startChunk; i < endChunk; ++i) DelChunk(body[i]);
        delete[] body;
    }


public:
    using iterator = baseIterator<T, Deque<T, chunk_size>>;
    using const_iterator = baseIterator<const T, const Deque<T, chunk_size>>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // chCap >= 1 => _begin.chunk >= 1 (otherwise SegFault during push_front, when do body[(_begin - 1).chunk] (_begin - 1).chunk = (size_t)(-1) )
    Deque(size_t size = 0) : elems(size), chCap((size + chunk_size - 1) / chunk_size + 1), sz(chCap * 3 + 2),
                             body(new T *[chCap * 3 + 2]),
                             _begin(iterator(chCap, 0, this)), _end(iterator(_begin + size)) {

        TakeMemory();
    }

    Deque(size_t size, const T &v) : Deque(size) {
        _end -= size; // _end = _begin + size (in Deque(size)
        for (size_t i = 0; i < size; ++i) {
            put(_end, v);
            ++_end;
        }
    }

    Deque(const Deque<T, chunk_size> &d) : elems(d.elems), chCap(d.chCap), sz(d.sz) {
        CreateDeque();
        for (Deque<T, chunk_size>::const_iterator it = d.begin(); it != d.end(); ++it) {
            put(_end, *it);
            ++_end;
        }
    }

    Deque<T, chunk_size> &operator=(const Deque<T, chunk_size> &d) {
        DelBody();
        elems = d.elems;
        chCap = d.chCap;
        sz = d.sz;
        CreateDeque();
        for (Deque<T, chunk_size>::const_iterator it = d.begin(); it != d.end(); ++it) {
            put(_end, *it);
            ++_end;
        }
        return *this;
    }

    ~Deque() noexcept {
        DelBody();
    }

    iterator begin() noexcept { return _begin; }

    const_iterator begin() const noexcept { return cbegin(); }

    iterator end() noexcept { return _end; }

    const_iterator end() const noexcept { return cend(); }
    const_iterator cbegin() const noexcept { return const_iterator(_begin); }

    const_iterator cend() const noexcept { return const_iterator(_end); }

    reverse_iterator rbegin() { return std::make_reverse_iterator(_end); }

    const_reverse_iterator rbegin() const { return const_iterator(rbegin()); }

    const_reverse_iterator crbegin() { return const_iterator(rbegin()); }

    reverse_iterator rend() { return std::make_reverse_iterator(_begin); }

    const_reverse_iterator rend() const { return const_iterator(rend()); }

    const_reverse_iterator crend() const { return const_iterator(rend()); }

    T &operator[](ssize_t n) {
        return *(_begin + static_cast<size_t>(n));
    }

    const T &operator[](ssize_t n) const {
        return *(_begin + static_cast<size_t>(n));
    }

    T &at(ssize_t n) {
        if (n < 0 || static_cast<size_t>(n) >= elems) throw std::out_of_range("Mimo");
        return *(_begin + static_cast<size_t>(n));
    }

    const T &at(ssize_t n) const {
        if (n < 0 || static_cast<size_t>(n) >= elems) throw std::out_of_range("Mimo");
        return *(_begin + static_cast<size_t>(n));
    }

    void push_back(const T &v) {
        if (sz == _end.chunk && _end.index == 0) resize(); // no free chunks & no free space in last chunk
        put(_end, v);
        ++_end;
        ++elems;
    }

    void push_front(const T &v) {
        if (0 == _begin.chunk && _begin.index == 0) resize(); // no free chunks & no free space in last chunk
        put(_begin - 1, v);
        --_begin;
        ++elems;
    }

    void pop_back() {
        if (_end == _begin) throw std::out_of_range("empty");
        (--_end)->~T();
        if (_end.index == chunk_size - 1) { // go to new chunk
            DelChunk(body[_end.chunk + 1]);
            body[_end.chunk + 1] = nullptr;
            --chCap;
        }
        --elems;
    }

    void pop_front() {
        if (_end == _begin) throw std::out_of_range("empty");
        _begin->~T();
        _begin++;
        if (_begin.index == 0) { // go to new chunk
            DelChunk(body[_begin.chunk - 1]);
            body[_begin.chunk - 1] = nullptr;
            --chCap;
        }
        --elems;
    }

    size_t size() const noexcept {
        return elems;
    }

    void erase(iterator it) {
        if (it < _begin || _end <= it) throw std::out_of_range("mimo");
        for (auto i = it; i < _end; ++i) {
            std::swap(*i, *(i + 1));
        }
        pop_back();
    }

    void insert(iterator it, const T &v) {
        if (it < _begin || _end < it) throw std::out_of_range("mimo");
        push_back(v);
        for (auto i = _end - 1; i > it; --i) {
            std::swap(*i, *(i - 1));
        }
    }


private:
    size_t elems;
    size_t chCap; // chunks allocated = (elems + chunk_size - 1) / chunk_size
    size_t sz; // external size
    T **body;
    iterator _begin;
    iterator _end;


    void put(iterator it, const T &v) {
        try {
            if (body[it.chunk] == nullptr) {
                body[it.chunk] = GetChunk();
                ++chCap;
            }
            new(&body[it.chunk][it.index]) T(v);
        } catch (...) {
            DelBody();
            throw;
        }
    }

    void TakeMemory() {
        size_t i = 0;
        try {
            for (; i < chCap; ++i)
                body[i] = nullptr;
            for (; i < (chCap << 1); ++i)
                body[i] = GetChunk();
            for (; i < chCap * 3 + 2; ++i)
                body[i] = nullptr;
        } catch (...) {
            for (size_t j = chCap; j < i; ++j) {
                delete[] body[i];
                delete[] body;
            }
            throw;
        }
    }

    void CreateDeque() {
        body = new T *[chCap * 3 + 2];
        TakeMemory();
        _begin = _end = iterator(chCap, 0, this); // don't do _end += size? because farther will use put(_end) & ++_end
    }

    void resize() {
        sz = chCap * 3;
        T **newArr = new T *[sz];
        memset(newArr, 0, sizeof(T *) * sz); // fill by nullptr
        memcpy(newArr + chCap, body + _begin.chunk, sizeof(T *) * chCap);
        delete[] body;
        body = newArr;
        _begin.chunk = chCap;
        _end.chunk = (chCap << 1) - 1;
    }
};

template<class T, size_t chunk_size>
template<typename ConstNonConstT, typename DequeT>
class Deque<T, chunk_size>::baseIterator : public std::iterator<std::random_access_iterator_tag, T> {

private:
    DequeT *orig = nullptr;
    size_t chunk = 0;
    size_t index = 0;

public:
    baseIterator() = default;

    baseIterator(size_t chunk, size_t index, Deque<T, chunk_size> *deque) : orig(deque), chunk(chunk), index(index) {}

    baseIterator(size_t chunk, size_t index, const Deque<T, chunk_size> *deque) : orig(
            const_cast<Deque<T, chunk_size> *>(deque)), chunk(chunk), index(index) {}

    baseIterator(const baseIterator<const T, const Deque<T, chunk_size>> &it) : orig(it.orig), chunk(it.chunk),
                                                                                index(it.index) {} // don't do explicit for implicit conversions from non_const to const

    baseIterator(const baseIterator<T, Deque<T, chunk_size>> &it) : orig(it.orig), chunk(it.chunk),
                                                                    index(it.index) {} // don't do explicit for implicit conversions from ...

    ~baseIterator() = default;

//    size_t GetChunk() const noexcept{ return chunk;}
//    size_t GetIndex() const noexcept{ return index;}
    friend Deque<T, chunk_size>;

    ConstNonConstT &operator*() const {
        return orig->body[chunk][index];
    }

    ConstNonConstT *operator->() const {
        return &orig->body[chunk][index];
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &operator++() noexcept {
        ++index;
        if (index == chunk_size) {
            ++chunk;
            index = 0;
        }
        return *this;
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> operator++(int) noexcept {
        Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> copy = *this;
        ++(*this);
        return copy;
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &operator--() noexcept {
        if (index == 0) {
            --chunk;
            index = chunk_size;
        }
        --index;
        return *this;
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> operator--(int) noexcept {
        Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> copy = *this;
        --(*this);
        return copy;
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &operator+=(long long n) noexcept {
        long long chS = static_cast<long long>(chunk_size);
        long long offset = n + index;
        if (offset >= 0 && offset < chS) {
            index += n;
        } else {
            long long shift;
            if (offset > 0) {
                shift = offset / chS;
            } else {
                shift = -(-offset - 1) / chS - 1;
            }
            chunk += shift;
            index = offset - shift * chS;
        }
        return *this;
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> operator+(long long n) const noexcept {
        Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> copy = *this;
        copy += n;
        return copy;
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &operator-=(long long n) noexcept {
        return *this += -n;
    }

    Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> operator-(long long n) const noexcept {
        Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> copy = *this;
        copy -= n;
        return copy;
    }

    int operator-(const Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &it) const noexcept {
        return (chunk - it.chunk) * chunk_size + index - it.index;
    }

    bool operator==(const Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &i) const noexcept {
        return (chunk == i.chunk) && (index == i.index);
    }

    bool operator!=(const Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &i) const noexcept {
        return !(*this == i);
    }

    bool operator<(const Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &i) const noexcept {
        return (chunk < i.chunk) || (chunk == i.chunk && index < i.index);
    }

    bool operator>(const Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &i) const noexcept {
        return (i < *this);
    }

    bool operator<=(const Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &i) const noexcept {
        return !(*this > i);
    }

    bool operator>=(const Deque<T, chunk_size>::baseIterator<ConstNonConstT, DequeT> &i) const noexcept {
        return !(*this < i);
    }

    friend std::conditional_t<std::is_same_v<Deque<T, chunk_size>, const Deque<T, chunk_size>>, baseIterator<T, Deque<T, chunk_size>>, baseIterator<const T, const Deque<T, chunk_size>>>;
};