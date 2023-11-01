// Copied from third party.
// Used to test the correctness of the program.
#pragma once

#include <string>
#include <format>
#include <iostream>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdlib>
#include <bits/allocator.h> // Used for constexpr allocator.

namespace dark {


/* Error code. */
struct error {
    std::string data;
    explicit error(std::string __s) noexcept : data(std::move(__s))
    { std::cerr << "\n\033[31mFatal error: " << data << "\n\033[0m"; }
    const char *what() const noexcept { return data.c_str(); }
};


/* Warning code. */
struct warning {
    std::string data;
    explicit warning(std::string __s) noexcept : data(std::move(__s))
    { std::cerr << "\033[33mWarning: " << data << "\n\033[0m"; }
    const char *what() const noexcept { return data.c_str(); }
};


/* Normal code. */
struct normal {
    std::string data;
    explicit normal(std::string __s) noexcept : data(std::move(__s))
    { std::cerr << "\033[32m" << data << "\n\033[0m"; }
    const char *what() const noexcept { return data.c_str(); }
};

} // namespace dark


namespace dark {

/* Basic construction part (Default/Copy/Move) */


template <class _Tp>
requires std::constructible_from <_Tp>
inline void construct(_Tp *__ptr)
noexcept(noexcept(::new (__ptr) _Tp()))
{ ::new(__ptr) _Tp(); }

template <class _Tp>
requires std::copy_constructible <_Tp>
inline void construct(_Tp *__ptr, const _Tp &__val)
noexcept(noexcept(::new(__ptr) _Tp(__val)))
{ ::new(__ptr) _Tp(__val); }

template <class _Tp>
requires std::move_constructible <_Tp>
inline void construct(_Tp *__ptr, _Tp &&__val)
noexcept(noexcept(::new(__ptr) _Tp(std::move(__val))))
{ ::new (__ptr) _Tp(std::move(__val)); }


/* Non-basic construction part. */


template <class _Tp,class _Up>
requires std::constructible_from <_Tp, _Up> && (!std::is_same_v <_Tp, _Up>)
inline void construct(_Tp *__ptr, _Up &&__val)
noexcept(noexcept(::new (__ptr) _Tp(std::forward <_Up>(__val))))
{ ::new (__ptr) _Tp(std::forward <_Up>(__val)); }

template <class _Tp,class ..._Args>
requires std::constructible_from <_Tp, _Args...>
inline void construct(_Tp *__ptr, _Args &&...__val)
noexcept(noexcept(::new (__ptr) _Tp(std::forward <_Args>(__val)...)))
{ ::new (__ptr) _Tp(std::forward <_Args>(__val)...); }


/* Destruction part. */

/**
 * @brief Special version for trivially destructible type.
 * It can be used to avoid the overhead of calling destructor.
 */
template <class _Tp>
requires std::is_trivially_destructible_v <_Tp>
inline void destroy([[maybe_unused]] _Tp *) noexcept {}

/**
 * @brief Special version for trivially destructible type.
 * It can be used to avoid the overhead of calling destructor.
 * It avoids a loop when the type is trivially destructible.
 */
template <class _Tp>
requires std::is_trivially_destructible_v <_Tp>
inline void destroy([[maybe_unused]] _Tp *, [[maybe_unused]] size_t) noexcept {}

/**
 * @brief Special version for trivially destructible type.
 * It can be used to avoid the overhead of calling destructor.
 * It avoids a loop when the type is trivially destructible.
 */
template <class _Tp>
requires std::is_trivially_destructible_v <_Tp>
inline void destroy([[maybe_unused]] _Tp *, [[maybe_unused]] _Tp *) noexcept {}

/**
 * @brief Destory the object pointed by __ptr.
 * @param __ptr Pointer to the object. 
 */
template <class _Tp>
requires (!std::is_trivially_destructible_v <_Tp>)
inline void destroy(_Tp *__ptr)
noexcept(std::is_nothrow_destructible_v <_Tp>)
{ __ptr->~_Tp(); }

/**
 * @brief Destroys the objects in the range [__ptr, __ptr + __n).
 * @param __ptr Pointer to the range of objects.
 * @param __n   Number of objects to destroy.
 */
template <class _Tp>
requires (!std::is_trivially_destructible_v <_Tp>)
inline void destroy(_Tp *__ptr, size_t __n)
noexcept(std::is_nothrow_destructible_v <_Tp>)
{ while(__n--) (__ptr++)->~_Tp(); }

/**
 * @brief Destroys the objects in the range [__ptr, __ptr + __n).
 * @param __ptr Pointer to the range of objects.
 * @param __n   Number of objects to destroy.
 */
template <class _Tp>
requires (!std::is_trivially_destructible_v <_Tp>)
inline void destroy(_Tp *__beg, _Tp *__end)
noexcept(std::is_nothrow_destructible_v <_Tp>)
{ while(__beg != __end) (__beg++)->~_Tp(); }


} // namespace dark


namespace dark {


#ifdef _DARK_DEBUG
class allocator_debugger;

class allocator_debug_helper {
  protected:
    struct debug_pack {
        allocator_debug_helper *pointer;
        size_t count;
    };

    friend class allocator_debugger;

    size_t refer_count = 0;
    size_t alloc_count = 0;
    size_t freed_count = 0;
    size_t alloc_times = 0;
    size_t freed_times = 0;

    inline static allocator_debug_helper *single = nullptr;
    inline static constexpr size_t offset = sizeof(debug_pack);

    allocator_debug_helper() noexcept {
        normal("Debug allocator is enabled!");
    }

    /* Aligned to offset. */
    static size_t pack_length(size_t __n) noexcept {
        return (__n + (offset - 1)) / offset + 2;
    }

    /* Aligned to offset. */
    static size_t real_size(size_t __n) noexcept {
        return pack_length(__n) * offset;
    }

    /* Perform some operation on allocated memory. */
    void *allocate(void *__ptr,size_t __n) {
        if(!__ptr) throw std::bad_alloc {};

        alloc_times += 1;
        alloc_count += __n;

        debug_pack __val = {this, __n};
        auto * const __tmp = static_cast <debug_pack *> (__ptr);
        *__tmp = __val;
        *(__tmp + pack_length(__n) - 1) = __val;
        return __tmp + 1;
    }

    /* Perform some operation before deallocation. */
    void *deallocate(void *__ptr) {
        if(!__ptr) return nullptr;
        auto *const __tmp = static_cast <debug_pack *> (__ptr) - 1;
        debug_pack __val = *__tmp;

        if (__val.pointer != this)
            throw error("Invalid deallocation!");

        auto *__end = __tmp + pack_length(__val.count) - 1;
        if (__end->pointer != this
        ||  __end->count != __val.count)
            throw error("Invalid deallocation!");

        freed_times += 1;
        freed_count += __val.count;

        return __tmp;
    }

    static allocator_debug_helper *get_object() {
        if(single == nullptr)
            single = new allocator_debug_helper {};
        ++single->refer_count;
        return single;
    }

    ~allocator_debug_helper() noexcept {
        auto __str = std::format (
                // "Memory leak detected!\n"
                "Allocated: {} bytes, {} times.\n"
                "Freed: {} bytes, {} times.\n"
                "Total loss: {} bytes, {} times.\n",
                alloc_count, alloc_times,
                freed_count, freed_times,
                alloc_count - freed_count,
                alloc_times - freed_times
            );
        if (alloc_count != freed_count || alloc_times != freed_times) {
            error("\nMemory leak detected!\n" + __str);
        } else {
            normal("\nNo memory leak is found!\n" + __str);
        }
    }
};

/* Debug allocator. It is a safer wrapper of debug_helper. */
class allocator_debugger {
  private:
    allocator_debug_helper *__obj;
  public:
    allocator_debugger() noexcept : 
        __obj(allocator_debug_helper::get_object()) {}
    ~allocator_debugger() noexcept { if(!--__obj->refer_count) delete __obj; }
    /* Aligned to offset. */
    void *allocate(size_t __n) {
        return __obj->allocate(::std::malloc(__obj->real_size(__n)),__n);
    }
    void deallocate(void *__ptr) noexcept {
        ::std::free(__obj->deallocate(__ptr));
    }
};

/* A safer wrapper of malloc and free. */
inline void *malloc(size_t __n) noexcept {
    static allocator_debugger __obj;
    return __obj.allocate(__n);
}

/* A safer wrapper of malloc and free. */
inline void free(void *__ptr) noexcept {
    static allocator_debugger __obj;
    return __obj.deallocate(__ptr);
}

/* A special class for debug use. */
template <class _Tp>
struct leaker {
    _Tp *ptr;

    leaker() : ptr((_Tp*) ::dark::malloc(sizeof(_Tp)))
    { ::dark::construct(ptr); }

    template <class _Up>
    requires std::constructible_from <_Tp, _Up> &&
        (!std::is_same_v <_Tp, std::decay_t <_Up>>)
    leaker(_Up &&__val) : ptr((_Tp*) ::dark::malloc(sizeof(_Tp))) {
        ::dark::construct(ptr,std::forward <_Up> (__val));
    }
    template <class ..._Args>
    requires std::constructible_from <_Tp, _Args...>
    leaker (_Args &&...__args) : ptr((_Tp*) ::dark::malloc(sizeof(_Tp))) {
        ::dark::construct(ptr,std::forward <_Args>(__args)...);
    }

    leaker(const leaker &__rhs) : leaker() {
        *ptr = *__rhs.ptr;
    }
    leaker(leaker &&__rhs) noexcept : ptr(__rhs.ptr) {
        __rhs.ptr = nullptr;
    }

    leaker &operator = (const leaker &__rhs) {
        *ptr = *__rhs.ptr;
        return *this;
    }
    leaker &operator = (leaker &&__rhs) noexcept {
        reset();
        ptr = __rhs.ptr;
        __rhs.ptr = nullptr;
        return *this;
    }

    void reset() noexcept { this->~leaker(); ptr = nullptr; }
    ~leaker() noexcept { if (ptr) { destroy(ptr); ::dark::free(ptr); } }

    bool operator == (const leaker &__rhs) const {
        return (!ptr && !__rhs.ptr) || (*ptr == *__rhs.ptr);
    }

    auto operator <=> (const leaker &__rhs) const {
        return *ptr <=> *__rhs.ptr;
    }
};

#else

using ::std::malloc;
using ::std::free;

#endif


/* A simple allocator. */
template <class _Tp>
struct allocator {
    inline static constexpr size_t __N = sizeof(_Tp);

    template <class U>
    struct rebind { using other = allocator<U>; };

    using size_type         = size_t;
    using difference_type   = ptrdiff_t;
    using value_type        = _Tp;
    using pointer           = _Tp *;
    using reference         = _Tp &;
    using const_pointer     = const _Tp*;
    using const_reference   = const _Tp&;

    [[nodiscard,__gnu__::__always_inline__]]
    constexpr static _Tp *allocate(size_t __n) {
        if consteval {
            return std::allocator <_Tp> {}.allocate(__n);
        } else {
            return static_cast <_Tp *> (::dark::malloc(__n * __N));
        }
    }

    [[__gnu__::__always_inline__]]
    constexpr static void deallocate(_Tp *__ptr,[[maybe_unused]] size_t __n)
    noexcept {
        if consteval {
            return std::allocator <_Tp> {}.deallocate(__ptr,__n);
        } else {
            return ::dark::free(__ptr);
        }
    }
};



} // namespace dark

