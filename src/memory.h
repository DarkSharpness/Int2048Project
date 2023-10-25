// Copied from third party.
// Used to test the correctness of the program.
#pragma once

#include <iostream>
#include <cstddef>
#include <utility>
#include <type_traits>


namespace dark {


struct error {
    std::string data;
    explicit error(error *) {}

    explicit error(std::string __s) : data(std::move(__s))
    { std::cerr << "\n\033[31mFatal error: " << data << "\n\033[0m"; }

    const char *what() const noexcept { return data.c_str(); }
};


struct warning {
    std::string data;
    explicit warning(warning *) {}

    warning(std::string __s) {
        std::cerr << "\033[33mWarning: " << __s << "\n\033[0m";
    }

    const char *what() const noexcept { return data.c_str(); }
};



}


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


#include <cstdlib>
#include <type_traits>
#include <utility>

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

    allocator_debug_helper() = delete;
    allocator_debug_helper(std::nullptr_t) noexcept {
        std::cerr << "\033[32mDebug allocator is enabled!\n\033[0m";
    }

    void *allocate(void *__beg,size_t __n) {
        if(!__beg) return nullptr;
        ++alloc_times;
        alloc_count += __n;

        void *__last = static_cast <char *> (__beg) + __n + offset * 2;
        debug_pack *__end = static_cast <debug_pack *> (__last) - 1;
        __end->pointer = this;
        __end->count   = __n;

        debug_pack *__tmp = static_cast <debug_pack *> (__beg);
        __tmp->pointer = this;
        __tmp->count   = __n;

        return __tmp + 1;
    }

    void *deallocate(void *__ptr) {
        if(!__ptr) return nullptr;
        ++freed_times;
        debug_pack *__tmp = static_cast <debug_pack *> (__ptr) - 1;
        if(__tmp->pointer != this) throw error("Deallocate Mismatch 1!");

        void *__beg = static_cast <void *> (__tmp);
        void *__last = static_cast <char *> (__beg) + __tmp->count + offset * 2;

        debug_pack *__end = static_cast <debug_pack *> (__last) - 1;
        if(__end->pointer != this) throw error("Deallocate Mismatch 2!");
        if(__end->count != __tmp->count) throw error("Deallocate Mismatch 3!");

        freed_count += __tmp->count;

        return __tmp;
    }

    static allocator_debug_helper *get_object() {
        if(single == nullptr) single = new allocator_debug_helper(nullptr);
        ++single->refer_count;
        return single;
    }

    ~allocator_debug_helper() noexcept(false) {
        if(alloc_count != freed_count || alloc_times != freed_times)
            throw error("Mismatched allocation and deallocation!\n" + 
                std::to_string(alloc_times) + " allocations, " +
                std::to_string(freed_times) + " deallocations!\n" + 
                "Totally " + std::to_string(alloc_count - freed_count) + " bytes leaked!");
        std::cerr << "\n\033[32mNo memory leak is found!\n\033[0m";
    }
};

/* Debug allocator (It will leak only a size of 16 bytes). */

class allocator_debugger {
  private:
    allocator_debug_helper *__obj;
  public:
    allocator_debugger() noexcept {
        __obj = allocator_debug_helper::get_object();
    }
    ~allocator_debugger() noexcept(false) {
        if(!--__obj->refer_count) delete __obj;
    }
    void *allocate(size_t __n) noexcept {
        return __obj->allocate(::std::malloc(__n + __obj->offset * 2),__n);
    }
    void deallocate(void *__ptr) noexcept {
        ::std::free(__obj->deallocate(__ptr));
    }
};

inline void *malloc(size_t __n) noexcept {
    static allocator_debugger __obj;
    return __obj.allocate(__n);
}

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
template <class T>
struct allocator {
    inline static constexpr size_t __N = sizeof(T);

    template <class U>
    struct rebind { using other = allocator<U>; };

    using size_type         = size_t;
    using difference_type   = ptrdiff_t;
    using value_type        = T;
    using pointer           = T *;
    using reference         = T &;
    using const_pointer     = const T*;
    using const_reference   = const T&;

    static T *allocate(size_t __n) { return static_cast <T *> (::dark::malloc(__n * __N)); }
    static void deallocate(T *__ptr, [[maybe_unused]] size_t __n) noexcept { ::dark::free(__ptr); }
};



} // namespace dark
