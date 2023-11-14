#pragma once

#include "int2048.h"
#include <numbers>

/* Implementation of arithmetic operators. */
namespace dark {

/**
 * @brief Convert a range to string.
 * @param __buf Buffer to store the result.
 * @param src Range of the number.
 * @note The input range cannot be empty!
 */
char *int2048_base::to_string(char *__buf,uint2048_view src) noexcept {
    auto [__beg,__end] = src;

    using namespace std::__detail;
    using namespace int2048_helper;

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
 * @brief Increase the range by 1, and store
 * the result to the given output iterator.
 * @param __ptr Begin of the output iterator.
 * @param src Range to be incremented.
 * @return Whether there is a carry.
 * @note Ensure that the output iterator has enough space.
 */
auto int2048_base::inc(_Iterator __ptr,uint2048_view src) noexcept -> inc_t {
    auto [__beg,__end] = src;
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
 * @param src Range to be decremented.
 * @return Whether there is be a vacancy.
 * @note Ensure that the output iterator has enough space.
 * If the input range is 0, the behavior is undefined!
 */
auto int2048_base::dec(_Iterator __ptr,uint2048_view src) noexcept -> dec_t {
    auto [__beg,__end] = src;
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
 * @brief Copy a range to another.
 * @param __ptr Output range.
 * @param src Input range.
 * @return End of the output range.
 */
auto int2048_base::cpy(_Iterator __ptr,uint2048_view src) noexcept -> cpy_t {
    const std::size_t _Length = src.size();
    if (__ptr != src.begin() && _Length)
        std::memmove(__ptr,src.begin(),_Length * sizeof(_Word_Type));
    return __ptr + _Length;
}

/**
 * @return The highest different base of two numbers and compare result.
 * @note Two input numbers should be equal in length.
 * None of these numbers is allowed to 0 in length.
 */
auto int2048_base::cmp(uint2048_view lhs,uint2048_view rhs) noexcept -> cmp_t {
    const auto __end = lhs.begin() - 1;
    auto __lhs = lhs.end();
    auto __rhs = rhs.end();
    do {
        if (*--__lhs != *--__rhs)
            return cmp_t {
                .length = static_cast <std::size_t> (__lhs - __end),
                .cmp    = *__lhs <=> *__rhs
            };
    } while (__lhs != __end);
    return cmp_t {0, std::strong_ordering::equal};
}

/**
 * @brief Add lhs and rhs to __ptr.
 * @param __ptr Output range.
 * @return Whether there is a carry.
 * @note lhs should be no shorter than rhs.
 */
auto int2048_base::add(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs)
noexcept -> add_t {
    bool __carry = 0;
    auto [__beg,__end] = lhs;
    for (const auto __cur : rhs) {
        const _Word_Type __sum = *(__beg++) + __cur + __carry;
        *__ptr++ = (__carry = __sum > Base - 1) ? __sum - Base : __sum;
    }

    if (__carry) {
        return inc(__ptr, {__beg,__end});
    } else {
        return cpy(__ptr, {__beg,__end}), false;
    }
}

/**
 * @brief Sub lhs and rhs to __ptr.
 * @param __ptr Output range.
 * @return Iterator to the tail of the result.
 * @note lhs should be strictly larger than rhs.
 */
auto int2048_base::sub(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs)
noexcept -> sub_t {
    bool __carry = 0;
    auto [__beg,__end] = lhs;
    for(const auto __cur : rhs) {
        const _Word_Type __sum = *(__beg++) - __cur - __carry;
        *__ptr++ = (__carry = __sum > Base - 1) ? __sum + Base : __sum;
    }

    if (__carry) {
        const auto __dist = __end - __beg;    
        // Example: 1000 - 999 = 0001 = 1
        if (__dist == 1 && *__beg == 1) {
            while (__ptr[-1] == 0) --__ptr; // Remove the leading 0.
        } else { // Example 10000 - 999 = 09001 = 9001
            __ptr += __dist - dec(__ptr, {__beg, __end});
        } return __ptr;
    } else {
        if (__beg != __end) { // Example 1999 - 999 = 1000
            __ptr = cpy(__ptr, {__beg, __end});
        } else { // Example: 2000 - 1999 = 0001 = 1
            while (__ptr[-1] == 0) --__ptr; // Remove the leading 0.
        } return __ptr;
    }
}

/**
 * @brief Multiply lhs and rhs to __ptr.
 * @param __ptr Output range.
 * @return Iterator to the tail of the result.
 */
auto int2048_base::mul(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs)
-> mul_t {
    if (use_brute_mul(lhs,rhs)) return brute_mul(__ptr,lhs,rhs);

    // We use decltype(auto) because we may return a reference.
    // Whether to use reference as optimization depends on implementation.
    decltype(auto) __fft = make_FFT(lhs , rhs);
    FFT_pass(__fft);

    auto __cpx  = __fft.begin();
    auto __cur  = _Word_Type {0};

    static_assert(FFT_Zip == 2, "Wrongly implemented!");
    for(std::size_t i = 0 ; i != __fft.size() ; ++i) {
        const _Word_Type __lo = std::llround(__cpx++->imag());
        const _Word_Type __hi = std::llround(__cpx++->imag());
        __cur   += __hi * FFT_Base + __lo;
        *__ptr++ = __cur % Base;
        __cur   /= Base;
    }

    if (__ptr[-1] == 0) --__ptr; // Remove the leading 0.
    return __ptr;
}

auto int2048_base::div(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs)
-> div_t {
    if (lhs.size() < rhs.size())    return __ptr;   // Of course 0.
    if (use_brute_div(lhs, rhs))    return brute_div(__ptr,lhs,rhs);

    _Container __buf {}; // Buffer. Maybe it should set static.
    uint2048_view __quotient = [&__buf, lhs, rhs]() {
        auto __shift = [](uint2048_view __v, std::size_t __n) {
            return uint2048_view {__v.begin() + __n, __v.end()};
        };
        // Now, delta is at least 1.
        const std::size_t _Delta = lhs.size() - rhs.size();
        if (_Delta <= rhs.size()) {
            __buf.init_capacity(lhs.size() + rhs.size() + 2);
            return __shift(try_div(__buf.begin(), lhs, rhs), rhs.size() * 2);
        } else {
            __buf.init_capacity(lhs.size() * 2 - rhs.size() + 2);
        }
    } ();

    return adjust_div(__ptr, __buf.begin(), lhs, rhs, __quotient);
}

/**
 * @brief Use newton method to give a fast and accurate division.
 * Error of this estimation is at most 1.
 * @param __beg Output range. (Custom buffer).
 * @return Range of the result.
 */
auto int2048_base::try_div(_Iterator __beg, uint2048_view lhs, uint2048_view rhs)
-> uint2048_view {
    _Iterator __tmp = inv(__beg, rhs);
    _Iterator __end = mul(__beg, lhs, {__beg, __tmp});
    return{ __beg,__end };
}

/**
 * @brief Work out the inverse of a number.
 * @param __ptr Output range.
 * @param __val Input number.
 * @return Iterator to the tail of the result.
 */
auto int2048_base::inv(_Iterator __ptr, uint2048_view __val)
noexcept -> inv_t {
    
}



} // namespace dark

/* Implementation of mul and div and mod. */
namespace dark {

/**
 * @return Whether lhs and rhs should be multiplied by brute force.
 * @note After this function, lhs will be no smaller than rhs in size.
 */
bool int2048_base::use_brute_mul(uint2048_view &lhs, uint2048_view &rhs) noexcept {
    if (lhs.size() < rhs.size()) std::swap(lhs,rhs);
    return lhs.size() < Max_Brute_Mul_Length;
}

/**
 * @brief Brute force multiplication.
 * @param __ptr Output range.
 * @return Iterator to the tail of the result.
 */
auto int2048_base::brute_mul(_Iterator __ptr, uint2048_view lhs, uint2048_view rhs)
noexcept -> mul_t {
    // First, we clear the memory for the result.
    // This requires that input ranges should not overlap with output range.
    std::memset(__ptr, 0, (lhs.size() + rhs.size()) * sizeof(_Word_Type));

    for (const auto __lhs : lhs) {
        _Iterator __tmp = __ptr;
        for (const auto __rhs : rhs) *__tmp++ += __lhs * __rhs;
        /* Operate on those low bits. */
        const auto __carry = *__ptr / Base;
        *__ptr %= Base;
        (void) ++__ptr;
        *__ptr += __carry;
    }

    _Word_Type __carry = 0;
    for ([[maybe_unused]] const auto __ : rhs) {
        __carry += *__ptr;
        *__ptr++ = __carry % Base;
        __carry /= Base;
    }

    if (__ptr[-1] == 0) --__ptr; // Remove the leading 0.
    return __ptr;
}

/**
 * @param _Max_Length The maximum length of the array.
 * @return Return the fft array generated by lhs and rhs.
 * Its capacity is the fft length.
 * Its size is the maximum length of the result.
 * @note lhs should be no smaller than rhs in size.
 */
auto int2048_base::make_FFT(uint2048_view lhs,uint2048_view rhs) -> FFT_t {
    static_assert(FFT_Zip == 2, "Wrongly implemented!");
    using namespace int2048_helper;

    const auto _Max_Length = lhs.size() + rhs.size();
    const auto _Bit_Length = 2 + __log2(_Max_Length - 1);
    const auto _Length     = std::size_t {1} << _Bit_Length;

    FFT_t __fft;
    __fft.init_capacity(_Length);
    __fft.resize(_Max_Length);

    auto *__ptr = __fft.begin();
    auto [__beg,__end] = lhs;

    for (const auto __rhs : rhs) {
        const auto __lhs = *__beg++;
        *__ptr++ = complex(__lhs % FFT_Base, __rhs % FFT_Base);
        *__ptr++ = complex(__lhs / FFT_Base, __rhs / FFT_Base);
    }
    while(__beg != __end) {
        const auto __val = *__beg++;
        *__ptr++ = complex(__val % FFT_Base);
        *__ptr++ = complex(__val / FFT_Base);
    }

    /* Clear the rest of the array. */
    std::memset((void *)__ptr, 0, (__fft.terminal() - __ptr) * sizeof(complex));
    return __fft;
}

/* Initialize by a given value. */
inline auto int2048_base::init_value(_Iterator __ptr, _Word_Type __val)
noexcept -> _Iterator {
    using namespace int2048_helper;
    static_assert(Word_Length == 3, "Wrongly implemented!");
    if (__val < __pow(Base,1)) {
        *__ptr++ = __val;
    } else if (__val < __pow(Base,2)) {
        *__ptr++ = __val % Base;
        *__ptr++ = __val / Base;
    } else {
        const auto __mid = __val / Base;
        *__ptr++ = __val % Base;
        *__ptr++ = __mid % Base;
        *__ptr++ = __mid / Base;
    } return __ptr;
}

/* Narrowing down to a builtin-type. */
inline auto int2048_base::narrow_down(_CIterator __ptr, std::size_t __len)
noexcept -> _Word_Type {
    using namespace int2048_helper;
    switch (__len) {
        case 0: return 0;
        case 1: return __ptr[0];
        case 2: return __ptr[0] + __ptr[1] * Base;
        default:
            return __ptr[0] + __ptr[1] * __pow(Base,1) + __ptr[2] * __pow(Base,2);
    }
}

bool int2048_base::use_brute_div(uint2048_view &lhs, uint2048_view &rhs) noexcept {
    const std::size_t _Delta = lhs.size() - rhs.size();
    return _Delta == 0 || (_Delta == 1 && *(lhs.end() - 1) < *(rhs.end() - 1));
}

auto int2048_base::brute_div(_Iterator __ptr, uint2048_view lhs, uint2048_view rhs)
-> div_t {
    auto __set_result = [__ptr](_Word_Type __val) -> _Iterator {
        *__ptr = __val; return __ptr + 1;
    }; // Simple function to set the result.

    const bool _Delta = lhs.size() - rhs.size();
    while(*rhs.begin() == 0) ++rhs._beg, ++lhs._beg;

    if (__builtin_expect(_Delta == 0 && lhs.size() == 1, false))
        return __set_result(*lhs.begin() / *rhs.begin());

    /* Estimate the possible range. */
    auto [__l , __r] = [=]() -> std::pair <std::size_t,std::size_t> {
        std::size_t __lhs = *(lhs.end() - 2) + *(lhs.end() - 1) * Base;
        std::size_t __rhs = *(rhs.end() - 1);
        if (_Delta == 1) __rhs = __rhs * Base + *(rhs.end() - 2);
        return { __lhs / (__rhs + 1), __lhs / __rhs + 1};
    } ();

    /* Now lhs < 2 * rhs, so return 0 or 1. */
    if (!__l) return __set_result(_Delta || cmp(lhs,rhs).cmp >= 0);

    _Container __buf { lhs.size() }; // Buffer. Maybe it should set static.

    /* Binary search in range [l , r)*/
    while (__l != __r) {
        const std::size_t __mid = (__l + __r) >> 1;

        auto __beg = __buf.begin();
        _Word_Type __carry = 0;
        for (auto __cur : rhs) {
            __carry += __cur * __mid;
            *__beg++ = __carry % Base;
            __carry /= Base;
        }

        if (_Delta == 0) {
            if (__carry)  { __r = __mid;     continue; }
        } else {
            if (!__carry) { __l = __mid + 1; continue; }
            *__beg++ = __carry;
        }

        auto __cmp = cmp({__buf.begin(),__beg} , lhs).cmp;
        if (__cmp == 0) return __set_result(__mid);
        else if (__cmp < 0)     __l = __mid + 1;
        else                    __r = __mid;
    } return __set_result(__l - 1);
}


} // namespace dark

/* Implementation of FFT base */
namespace dark {

/**
 * @brief The implementation of FFT.
 * @param __cpx Input complex array, which will be modified.
 * @param __len Length of the array.
 * @note Length of the array should be a power of 2.
 * In addition, the length should be at least 4.
 */
void FFT_base::FFT(complex * __cpx, std::size_t __len) noexcept {
    /* Swap accoring to bit-reverse  */
    for (std::size_t i = 1, j = __len >> 1; i != __len ; ++i) {
        if (i < j) std::swap(__cpx[i],__cpx[j]);
        std::size_t k = __len; // Magic!
        do { k >>= 1; } while ((j ^= k) < k);
    }

    /* Calculate the unit root. */
    auto [__table,__bits] = make_table(__len);

    std::size_t i = 1;
    do { /* Core FFT */
        --__bits;
        for (std::size_t j = 0 ; j != __len ; j += (i << 1)) {
            for (std::size_t k = 0 ; k != i ; ++k) {
                auto &__lhs = __cpx[j | k];
                auto &__rhs = __cpx[j | k | i];
                auto  __tmp = __rhs * __table[k << __bits];
                __rhs = __lhs - __tmp;
                __lhs = __lhs + __tmp;
            }
        }
    } while((i <<= 1) != __len);
}

/**
 * @brief Make the root table for FFT.
 * @param __len Length of the root table.
 * @return Custom information struct.
 */
auto FFT_base::make_table(std::size_t __len) -> table_t {
    using namespace int2048_helper;
    static vector <complex> __table {};
    if (__table.size() < __len) {
        __table.~vector();
        __table.init_capacity(__len);
        __table.resize(__len);
        __table[0] = {1.0, 0.0};

        auto __beg = __table.begin();
        auto __end = __table.end();

        std::size_t __cnt = 0;
        const double __delta = 2.0 * std::numbers::pi / __len;

        do { /* Avoid too many function calls. */
            double __angle = __delta * ++__cnt;

            const double __cos = std::cos(__angle);
            const double __sin = std::sin(__angle);

            *++__beg = {__cos, __sin};
            *--__end = {__cos,-__sin};
        } while (__beg != __end);

        *__beg = {-1.0 , 0.0}; // cos(pi), sin(pi)
    }

    return {__table.begin(), __log2(__table.size())};
}

/**
 * @brief Merge the result of 2 FFT.
 * @param __beg Begin of the array.
 * @param __end End of the array.
 */
inline void FFT_base::merge_FFT(complex *__beg, complex *__end) noexcept {
    const double __mul = 0.5 / (__end - __beg);
    while (__beg != __end) {
        auto &__cur = *__beg++;
        __cur = (__cur * __cur) * __mul;
    }
}

/**
 * @brief Do the work of IFFT to a given array.
 * @param __beg Begin of the array.
 * @param __end End of the array.
 */
inline void FFT_base::final_FFT(complex *__beg, complex *__end) noexcept {
    while (++__beg != --__end) std::swap(*__beg , *__end);
}


/**
 * @brief A pass through all things to do in FFT.
 * @param __fft FFT array generated by make_FFT.
 */
inline void FFT_base::FFT_pass(FFT_t &__fft) noexcept {
    FFT (__fft.begin(), __fft.capacity());
    merge_FFT(__fft.begin(), __fft.terminal());
    FFT (__fft.begin(), __fft.capacity());
    final_FFT(__fft.begin(), __fft.terminal());
}


} // namespace dark

