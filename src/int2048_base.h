#pragma once

#include "int2048.h"
#include <bit>

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
int2048_base::inc_t
    int2048_base::inc(_Iterator __ptr,uint2048_view __int) noexcept {
    auto __beg = __int.begin();
    auto __end = __int.end();
    while (__beg != __end) {
        const _Word_Type __cur = (*__beg++) + 1;
        if (__builtin_expect(__cur < Base,true)) {
            *__ptr++ = __cur;
            return (cpy(__ptr,{__beg,__end}) , false);
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
int2048_base::dec_t
    int2048_base::dec(_Iterator __ptr,uint2048_view __int) noexcept {
    auto __beg = __int.begin();
    auto __end = __int.end();
    while (__beg != __end) {
        const _Word_Type __cur = (*__beg++) - 1;
        if (__builtin_expect(__cur < Base,true))  {
            *__ptr++ = __cur;
            return __beg == __end ?
                (__cur == 0) : (cpy(__ptr,{__beg,__end}) , false);
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
 * @brief Copy a range to another.
 * @param __ptr Output range.
 * @return End of the output range.
 */
int2048_base::cpy_t
    int2048_base::cpy(_Iterator __ptr,uint2048_view __int) noexcept {
    const std::size_t _Length = __int.size();
    if (__ptr != __int.begin() && _Length)
        std::memmove(__ptr,__int.begin(),_Length * sizeof(_Word_Type));
    return __ptr + _Length;
}

/**
 * @return The highest different base of two numbers and compare result.
 * @note Two input numbers should be equal in length.
 * None of these numbers is allowed to 0 in length.
 */
int2048_base::cmp_t
    int2048_base::cmp(uint2048_view lhs,uint2048_view rhs) noexcept {
    const auto __end = lhs.begin() - 1;
    auto __lhs = lhs.end();
    auto __rhs = rhs.end();
    do {
        if (*--__lhs != *--__rhs)
            return {
                static_cast <std::size_t> (__lhs - __end),
                *__lhs <=> *__rhs
            };
    } while (__lhs != __end);
    return {0, std::strong_ordering::equal};
}

/**
 * @brief Add lhs and rhs to __ptr.
 * @param __ptr Output range.
 * @return Whether there is a carry.
 * @note lhs should be no shorter than rhs.
 */
int2048_base::add_t
    int2048_base::add(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs) noexcept {
    bool __carry = 0;
    auto __lhs = lhs.begin();
    for (const auto __cur : rhs) {
        const _Word_Type __sum = *(__lhs++) + __cur + __carry;
        *__ptr++ = (__carry = __sum > Base - 1) ? __sum - Base : __sum;
    }

    if (__carry) {
        return inc(__ptr, {__lhs,lhs.end()});
    } else {
        return cpy(__ptr, {__lhs,lhs.end()}), false;
    }
}

/**
 * @brief Sub lhs and rhs to __ptr.
 * @param __ptr Output range.
 * @return Iterator to the tail of the result.
 * @note lhs should be strictly larger than rhs.
 */
int2048_base::sub_t
    int2048_base::sub(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs) noexcept {
    bool __carry = 0;
    auto __lhs = lhs.begin();
    for(const auto __cur : rhs) {
        const _Word_Type __sum = *(__lhs++) - __cur - __carry;
        *__ptr++ = (__carry = __sum > Base - 1) ? __sum + Base : __sum;
    }

    if (__carry) {
        const auto __dist = lhs.end() - __lhs;    
        if (__dist == 1 && *__lhs == 1) {
            while (*(__ptr - 1) == 0) --__ptr; // Remove the leading 0.
        } else {
            __ptr += __dist - dec(__ptr, {__lhs,lhs.end()});
        } return __ptr;
    } else {
        if (__lhs != lhs.end()) {
            __ptr = cpy(__ptr, {__lhs,lhs.end()});
        } else {
            while (*(__ptr - 1) == 0) --__ptr; // Remove the leading 0.
        } return __ptr;
    }
}


/**
 * @brief Mul lhs and rhs to __ptr.
 * @param __ptr Output range.
 * @return Iterator to the tail of the result.
 */
int2048_base::mul_t
    int2048_base::mul(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs) noexcept {
    if (is_brute_mulable(lhs,rhs)) return brute_mul(__ptr,lhs,rhs);
    /* Core: FFT multiplication. */

    const auto _Max_Length  = lhs.size() + rhs.size(); // It should be no less than 2.
    const auto _Bit_Length  = 32 - std::countl_zero <std::uint32_t> (_Max_Length - 1);
    const auto _Length      = std::size_t {1} << _Bit_Length;

    auto __fft = make_FFT(lhs,rhs,_Length);

}

int2048_base::FFT_t
    int2048_base::make_FFT(uint2048_view lhs,uint2048_view rhs,std::size_t __len) noexcept {
    FFT_t __fft; __fft.reserve(__len);

    if (lhs.size() < rhs.size()) std::swap(lhs,rhs);
    auto __lhs = lhs.begin();

    /* Clear rhs and then clear lhs. */
    for (const auto __cur : rhs) __fft.push_back({*__lhs++,__cur});
    while(__lhs != lhs.end())    __fft.push_back({*__lhs++,  0  });

    /* Fill the reset with 0. */
    __fft.resize(__len);
    return __fft;
}


} // namespace dark
