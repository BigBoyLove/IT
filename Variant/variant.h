
#include <type_traits>
#include <initializer_list>
#include <exception>
#include <memory>
#include <climits>

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wreorder"

const size_t NPOS = ULLONG_MAX;

template<size_t N, typename T, typename... Tail>
struct get_index_by_type_impl {
    static const size_t value = NPOS;
};
template<size_t N, typename T, typename Head, typename... Tail>
struct get_index_by_type_impl<N, T, Head, Tail ...> {
    static constexpr size_t value = std::is_same_v<T, Head> ? N : get_index_by_type_impl<N + 1, T, Tail...>::value;
};

/// return N by type T & Types
/// if T in Types
/// NPOS if not
template<typename T, typename... Types>
static constexpr size_t get_index_by_type = get_index_by_type_impl<0, T, Types...>::value;

template<size_t N, typename Head, typename... Tail>
struct get_type_by_index_impl {
    using type = std::conditional_t<N == 0, Head, typename get_type_by_index_impl<N - 1, Tail...>::type>;
};
template<size_t N, typename Head>
struct get_type_by_index_impl<N, Head> {
    using type = Head;
};

/// return T by index N & Types if N < sizeof...(Types)
template<size_t N, typename... Types>
using get_type_by_index = typename get_type_by_index_impl<N, Types...>::type;


template<size_t N, typename T, typename... Tail>
struct get_index_by_type_if_constructable_impl {
    static constexpr size_t value = NPOS;
};
template<size_t N, typename T, typename Head, typename... Tail>
struct get_index_by_type_if_constructable_impl<N, T, Head, Tail ...> {
    static constexpr size_t value = std::is_constructible_v<Head, T> ? N : get_index_by_type_if_constructable_impl<
            N + 1, T, Tail...>::value;
};
/// return index by T & Types
/// if T constructable ...(Types)
/// NPOS if not
template<typename T, typename... Types>
static constexpr size_t get_index_by_type_if_constructable = get_index_by_type_if_constructable_impl<0, T, Types...>::value;


template<size_t N, typename T, typename ... Tail>
struct is_only_one_constructable_impl {
    static constexpr bool value = N == 1;
};
template<size_t N, typename T, typename Head, typename... Tail>
struct is_only_one_constructable_impl<N, T, Head, Tail... > {
    static constexpr bool value = is_only_one_constructable_impl<
            static_cast<size_t>(std::is_constructible_v<Head, T>) + N, T, Tail...>::value;
};
template<typename T, typename... Types>
static constexpr bool is_only_one_constructable = is_only_one_constructable_impl<0, T, Types...>::value;


template<typename Head, typename... Tail>
union VariadicUnion {
    Head head;
    VariadicUnion<Tail...> tail;

    VariadicUnion() {}

    ~VariadicUnion() {}

    template<size_t N>
    const auto &get() const noexcept {
        if constexpr (N == 0) {
            return head;
        } else {
            return tail.template get<N - 1>();
        }
    }

    template<size_t N>
    auto &get() noexcept {
        if constexpr (N == 0) {
            return head;
        } else {
            return tail.template get<N - 1>();
        }
    }

    template<typename T, size_t N, typename... Args>
    void put(Args &&... args) {
        if constexpr (N == 0) {
            new((void *) std::launder(&head)) T(std::forward<Args>(args)...);
        } else {
            tail.template put<T, N - 1>(std::forward<Args>(args)...);
        }
    }

    template<size_t N>
    void destroy() noexcept {
        if constexpr (N == 0) {
            head.~Head();
        } else {
            tail.template destroy<N - 1>();
        }
    }
};

template<typename Head>
union VariadicUnion<Head> { // specialization
    Head head;

    VariadicUnion() {}

    ~VariadicUnion() {}

    template<size_t N>
    const auto &get() const noexcept {
        static_assert(N == 0, "no alternative");
        return head;
    }

    template<size_t N>
    auto &get() noexcept {
        static_assert(N == 0, "no alternative");
        return head;
    }

    template<typename T, size_t N, typename... Args>
    void put(Args &&... args) {
        static_assert(N == 0, "no alternative");
        new((void *) std::launder(&head)) T(std::forward<Args>(args)...);
    }

    template<size_t N>
    void destroy() noexcept {
        static_assert(N == 0, "no alternative");
        head.~Head();
    }

};

template<typename... Types>
class Variant;

template<typename T, typename... Types>
struct VariantAlternative {
    using Derived = Variant<Types...>;

    static constexpr size_t index = get_index_by_type<T, Types...>;

    VariantAlternative() = default;

    VariantAlternative(const VariantAlternative &other) {
        auto pOther = static_cast<const Derived *>(&other);
        if (pOther->current != other.index) return; // not active alternative

        tryPut(static_cast<Derived *>(this), pOther->storage.template get<index>());
    }

    VariantAlternative(VariantAlternative &&other) noexcept {
        auto pOther = static_cast<Derived *>(&other);
        if (pOther->current != other.index) return; // not active alternative

        put(static_cast<Derived *>(this), std::move(pOther->storage.template get<index>()));
    }

    VariantAlternative(const T &value) {
        tryPut(static_cast<Derived *>(this), value);
    }

    VariantAlternative(T &&value) noexcept {
        put(static_cast<Derived *>(this), std::move(value));
    }

    ~VariantAlternative() = default;

    Derived &operator=(const T &value) {
        auto pVariant = static_cast<Derived *>(this);
        if (pVariant->current != index) pVariant->destroy();

        tryPut(pVariant, value);
        return *pVariant;
    }

    Derived &operator=(T &&value) noexcept {
        auto pVariant = static_cast<Derived *>(this);
        if (pVariant->current != index) pVariant->destroy();

        put(pVariant, std::move(value));
        return *pVariant;

    }

    void destroy() noexcept {
        auto pVariant = static_cast<Derived *>(this);
        if (index != pVariant->current) return; // not active alternative

        pVariant->storage.template destroy<index>();
    }

private:
    static void put(Derived *p, T &&value) {
        p->storage.template put<T, index>(std::forward<T>(value));
        p->current = index;
    }

    static void tryPut(Derived *p, const T &value) {
        try {
            p->storage.template put<T, index>(value);
        } catch (...) {
            p->current = NPOS;
            throw;
        }
        p->current = index;
    }
};

template<typename T, typename... Types>
T &get(Variant<Types...> &);

template<typename T, typename... Types>
const T &get(const Variant<Types...> &);



template<typename... Types>
class Variant : private ::VariantAlternative<Types, Types...> ... {
public:
    Variant() : current(0) {
        storage.template put<get_type_by_index<0, Types...>, 0>();
    }

    Variant(const Variant &other)
            : current(0), ::VariantAlternative<Types, Types...>(other)... {} // unpacking

    Variant(Variant &&other) noexcept
            : current(0), ::VariantAlternative<Types, Types...>(std::move(other))... {} // unpacking

    template<typename T, typename = std::enable_if_t<is_only_one_constructable<T, Types...>>>
    Variant(T &&value) {
        constexpr size_t newTypeId = get_index_by_type_if_constructable<T, Types...>;
        Variant::tryPut<::get_type_by_index<newTypeId, Types...>, newTypeId, T>(std::forward<T>(value));
    }

    template<typename T, typename = std::enable_if_t<std::is_constructible_v<Variant, T> && is_only_one_constructable<T, Types...> > >
    Variant &operator=(T &&value) {
        constexpr size_t newTypeId = get_index_by_type_if_constructable<T, Types...>;
        Variant::destroyIfChanged(newTypeId);

        Variant::tryPut<get_type_by_index<newTypeId, Types...>, newTypeId, T>(std::forward<T>(value));
        return *this;
    }
    Variant &operator=(const Variant &other) {
        Variant::destroyIfChanged(other.current);
        (Variant::assignmentHelp(::VariantAlternative<Types, Types...>(), other), ...); // shrinkwrap to apply the function to the desired VariantAlternative
        return *this;
    }

    Variant &operator=(Variant &&other) noexcept {
        Variant::destroyIfChanged(other.current);
        (Variant::moveAssignmentHelp(::VariantAlternative<Types, Types...>(), other), ...); // shrinkwrap to apply the function to the desired VariantAlternative
        other.destroy();
        return *this;
    }


    ~Variant() {
        Variant::destroy();
    }

    template<typename T, typename... Args>
    T &emplace(Args &&... args) {
        Variant::destroy();

        Variant::tryPut<T, get_index_by_type<T, Types...>, Args...>(std::forward<Args>(args)... );
        return get<T>(*this);
    }

    template<size_t N,typename = std::enable_if_t<N < sizeof...(Types) >, typename... Args>
    auto &emplace(Args &&... args) {
        return Variant::emplace<get_type_by_index<N, Types...>>(std::forward<Args>(args)...);
    }

    template<typename T, typename U, typename... Args>
    T &emplace(std::initializer_list<U> list, Args &&... args) {
        Variant::destroy();

        Variant::tryPut<T, get_index_by_type<T, Types...>, Args...>(list, std::forward<Args>(args)... );
        return get<T>(*this);
    }

    template<size_t N, typename = std::enable_if_t<N < sizeof...(Types) >, typename U, typename... Args>
    auto &emplace(std::initializer_list<U> list, Args &&... args) {
        return Variant::emplace<get_type_by_index<N, Types...>>(list, std::forward<Args>(args)...);
    }

    size_t index() const noexcept {
        return current;
    }

    bool valueless_by_exception() const noexcept {
        return current == NPOS;
    }

private:
    size_t current;
    VariadicUnion<Types...> storage;

    template<typename SearchType, size_t typeIndex, typename ... Args>
    void tryPut( Args &&... args ) {
        try {
            storage.template put<SearchType, typeIndex>(std::forward<Args>(args)...);
        } catch (...) {
            current = NPOS;
            throw;
        }
        current = typeIndex;
    }

//    template<typename SearchType, size_t typeIndex, typename ... Args>
//    void put( Args &&... args ) {
//        storage.template put<SearchType, typeIndex>(std::forward<Args>(args)...);
//        current = typeIndex;
//    }

    void destroy() noexcept {
        (::VariantAlternative<Types, Types...>::destroy(), ...);
    }

    void destroyIfChanged(size_t new_alternative) noexcept {
        if (new_alternative == current) return;
        Variant::destroy();
    }

    template<typename T>
    void assignmentHelp( const VariantAlternative<T, Types...> &alternative, const Variant &other) {
        if (other.current != alternative.index) return;
        constexpr size_t alternativeType = get<alternative.index>;

        Variant::tryPut<get_type_by_index<alternative.index, Types...>, alternative.index, alternativeType>( alternativeType(other) );
    }

    template<typename T>
    void moveAssignmentHelp(const VariantAlternative<T, Types...> &alternative, Variant &other) noexcept {
        if (other.current != alternative.index) return;
        storage.template put<get_type_by_index<alternative.index, Types...>, alternative.index>(std::move(get<alternative.index>(other)));
        current = other.current;
    }

    template<typename T, typename... Ts>
    friend struct VariantAlternative;

    template<typename T, typename... Ts>
    friend bool holds_alternative(const Variant<Ts...> &) noexcept;

    template<size_t N, typename... Ts>
    friend const auto &get(const Variant<Ts...> &);

    template<size_t N, typename... Ts>
    friend auto &get(Variant<Ts...> &);

    template<size_t N, typename... Ts>
    friend const auto &&get(const Variant<Ts...> &&);

    template<size_t N, typename... Ts>
    friend auto &&get(Variant<Ts...> &&);

//    using VariantAlternative<Types, Types...>::operator=...;

    using VariantAlternative<Types, Types...>::VariantAlternative...;
};

template<typename T, typename... Types>
bool holds_alternative(const Variant<Types...> &variant) noexcept {
    return get_index_by_type<T, Types...> == variant.current;
}

template<size_t N, typename... Types>
const auto &get(const Variant<Types...> &variant) {
    if (N != variant.current) throw std::bad_variant_access();
    return variant.storage.template get<N>();
}

template<size_t N, typename... Types>
auto &get(Variant<Types...> &variant) {
    if (N != variant.current) throw std::bad_variant_access();
    return variant.storage.template get<N>();
}

template<size_t N, typename... Types>
const auto &&get(const Variant<Types...> &&variant) {
    if (N != variant.current) throw std::bad_variant_access();
    return variant.storage.template get<N>();
}

template<size_t N, typename... Types>
auto &&get(Variant<Types...> &&variant) {
    if (N != variant.current) throw std::bad_variant_access();
    return variant.storage.template get<N>();
}

template<typename T, typename... Types>
const T &get(const Variant<Types...> &variant) {
    return get<get_index_by_type<T, Types...>>(variant);
}

template<typename T, typename... Types>
T &get(Variant<Types...> &variant) {
    return get<get_index_by_type<T, Types...>>(variant);
}

template<typename T, typename... Types>
const T &&get(const Variant<Types...> &&variant) {
    return get<get_index_by_type<T, Types...>>(std::move(variant));
}

template<typename T, typename... Types>
T &&get(Variant<Types...> &&variant) {
    return get<get_index_by_type<T, Types...>>(std::move(variant));
}
