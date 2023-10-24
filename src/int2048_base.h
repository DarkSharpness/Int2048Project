#pragma once
#include "int2048.h"

/* Implementation of int2048 base. */
namespace dark {

/**
 * @brief Increase the range by 1, and store
 * the result to the given output iterator.
 * @param __ptr Begin of the output iterator.
 * @param __int Range to be incremented.
 * @return Whether there is a carry.
 * @note Ensure that the output iterator has enough space.
 */
bool int2048_base::increment(_Iterator __ptr,uint2048_view __int) noexcept {
    for(const _Word_Type __it : __int) {
        const _Word_Type __cur = __it + 1;
        if (__builtin_expect(__cur < Base,true)) {
            *__ptr = __cur; return false;
        } else {
            *__ptr++ = 0;
        }
    } return true;
}


/**
 * @brief Decrease the range by 1, and store
 * the result to the given output iterator.
 * @param __ptr Begin of the output iterator.
 * @param __int Range to be decremented.
 * @return Whether there is be a vacancy.
 * @note Ensure that the output iterator has enough space.
 * If the input range is 0, the behavior is undefined!
 */
bool int2048_base::decrement(_Iterator __ptr,uint2048_view __int) noexcept {
    auto __beg = __int.begin();
    auto __end = __int.end();
    while (__beg != __end) {
        const _Word_Type __cur = (*__beg++) - 1;
        if (__builtin_expect(__cur < Base,true))  {
            return (*__ptr = __cur) == 0 && __beg == __end;
        } else {
            *__ptr++ = Base - 1;
        }
    } __builtin_unreachable(); // Iff input range is 0.
}


/**
 * @brief Convert a range to string.
 * @param __buf Buffer to store the result.
 * @param __int Range of the number.
 * @note The input range cannot be empty!
 */
char *int2048_base::to_string(char *__buf,uint2048_view __int) noexcept {
    auto __beg = __int.begin(); /* Low  part. */
    auto __end = __int.end();   /* High part. */

    using namespace std::__detail;

    std::size_t __len = __to_chars_len(*--__end);
    __to_chars_10_impl(__buf,__len,*__end);
    __buf += __len;

    while(__end-- != __beg) {
        _Word_Type  __cur = *__end;
        std::size_t __len = Base_Length;
        while (__len --> 0) { // Magic --> operator
            __buf[__len] = make_char(__cur % 10);
            __cur /= 10;
        }
        __buf += Base_Length;
    }

    return __buf; // Return the end of the string.
}


/**
 * @return The highest different base of two numbers and compare result.
 * @note Make sure that the two numbers are equal in length.
 * Make sure that none of the two numbers are 0.
 */
int2048_base::cmp_t
    int2048_base::cmp(uint2048_view lhs,uint2048_view rhs) noexcept {
    const auto __end = lhs.begin();
    auto __lhs = lhs.end();
    auto __rhs = rhs.end();
    while (__lhs != __end) {
        if (*--__lhs != *--__rhs)
            return {
                static_cast <std::size_t> (__lhs - __end),
                *__lhs <=> *__rhs
            };
    } return {static_cast <std::size_t> (-1),std::strong_ordering::equal};
}

int2048_base::add_t
    int2048_base::add(_Iterator __beg,uint2048_view lhs,uint2048_view rhs) noexcept {
    __builtin_unreachable();
}

int2048_base::sub_t
    int2048_base::sub(_Iterator __beg,uint2048_view lhs,uint2048_view rhs) noexcept {
    __builtin_unreachable();
}

} // namespace dark
