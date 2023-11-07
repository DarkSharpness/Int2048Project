#pragma once

#include "int2048.h"
#include <bit>

/* Implementation of int2048 base. */
namespace dark {

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
 * @brief Convert a range to string.
 * @param __buf Buffer to store the result.
 * @param src Range of the number.
 * @note The input range cannot be empty!
 */
char *int2048_base::to_string(char *__buf,uint2048_view src) noexcept {
    auto [__beg,__end] = src;

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
auto int2048_base::add(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs) noexcept -> add_t {
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
auto int2048_base::sub(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs) noexcept -> sub_t {
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
auto int2048_base::mul(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs) noexcept -> mul_t {
    // if (is_brute_mulable(lhs,rhs)) return brute_mul(__ptr,lhs,rhs);

    const auto _Max_Length = lhs.size() + rhs.size();
    // We use decltype(auto) because we may return a reference.
    // Whether to use reference as optimization depends on implementation.
    decltype(auto) __fft = make_fft(lhs , rhs, _Max_Length);
    decltype(auto) __rev = make_rev(__fft.size());

    swap_rev(__fft.begin(), __rev.begin() , __fft.size());
    FFT_base::FFT (__fft.begin() , __fft.size());

    for(auto &__val : __fft) __val *= __val;

    swap_rev(__fft.begin(), __rev.begin(), __fft.size());
    FFT_base::IFFT(__fft.begin() , __fft.size());

    auto __cpx = __fft.begin();

    _Word_Type __carry = 0;
    for(std::size_t i = 0 ; i != _Max_Length ; ++i) {
        const _Word_Type __lo = std::llround((__cpx++)->imag());
        const _Word_Type __hi = std::llround((__cpx++)->imag());
        __carry += __hi * FFT_Base + __lo;
        *__ptr++ = __carry % Base;
        __carry /= Base;
    }

    while (*(__ptr - 1) == 0) --__ptr; // Remove the leading 0.
    return __ptr;
}

namespace int2048_helper {

constexpr auto __log2(std::size_t __val) -> std::size_t {
    return 31 - std::countl_zero <std::uint32_t> (__val);
}

constexpr auto __fdiv(double __val, std::size_t __shift) -> double {
    struct { /* Only works in small endian machines! */
        using ull = std::size_t;
        static_assert(sizeof(double) == sizeof(ull));
        static_assert(sizeof(double) == 8);
        union {
            double dat;
            struct { /* A reference. */
                ull frac : 52;
                ull exp  : 11;
                ull sign : 1;
            };
            ull raw;
        };
    } __tmp = {__val};
    if (__tmp.raw) __tmp.raw -= __shift << 52;
    // __tmp.exp -= __shift;
    return __tmp.dat;
}


} // namespace int2048_helper


auto int2048_base::make_fft(
    uint2048_view lhs,
    uint2048_view rhs,
    const std::size_t _Max_Length) -> fft_t {
    using namespace int2048_helper;

    const auto _Bit_Length  = 2 + __log2(_Max_Length - 1);
    const auto _Length      = std::size_t {1} << _Bit_Length;

    fft_t __fft; __fft.init_capacity(_Length);

    if (lhs.size() < rhs.size()) std::swap(lhs , rhs);
    auto __lhs = lhs.begin();

    const auto _LShift = 1 + __log2(lhs.size());
    const auto _Rshift = _Bit_Length + 1 - _LShift;

    /* Visit rhs and then visit lhs. */
    for (const auto __cur : rhs) {
        const auto __val = *__lhs++;
        __fft.emplace_back(
            __fdiv(__val % FFT_Base , _LShift),
            __fdiv(__cur % FFT_Base , _Rshift));
        __fft.emplace_back(
            __fdiv(__val / FFT_Base , _LShift),
            __fdiv(__cur / FFT_Base , _Rshift));
    }

    while(__lhs != lhs.end()) {
        const auto __val = *__lhs++;
        __fft.emplace_back(__fdiv(__val % FFT_Base , _LShift));
        __fft.emplace_back(__fdiv(__val / FFT_Base , _LShift));
    }

    __fft.fill_size(_Length);
    return __fft;
}


/**
 * @brief Make the reverse array.
 * @param __len Length of the array.
 * @return Reverse array.
 * @note __len should be no less than 2.
 */
auto int2048_base::make_rev(std::size_t __len) -> rev_t {
    rev_t __rev; __rev.init_capacity(__len);
    const auto __half = __len >> 1;

    __rev.push_back(0);
    __rev.push_back(__half);

    for(std::size_t i = 1 ; i < __half; ++i) {
        const auto __cur = __rev[i] >> 1;
        __rev.push_back(__cur);
        __rev.push_back(__cur | __half);
    }

    return __rev;
}


/**
 * @brief Swap according to the reverse array.
 * @param __cpx FFT array to be swapped.
 * @param __rev Reverse array.
 * @param __len Length of the FFT array.
 */
void int2048_base::swap_rev(complex *__cpx,const idx_t *__rev, std::size_t __len) noexcept {
    for (std::size_t i = 0; i < __len; ++i)
        if (i < __rev[i]) // Avoid swap twice.
            std::swap(__cpx[i], __cpx[__rev[i]]);
}


} // namespace dark

/* Implementation of FFT base */
namespace dark {

namespace int2048_helper {

template <bool _Is_IFFT>
inline void __FFT_impl(FFT_base::complex * __cpx, std::size_t __len) noexcept {
    std::size_t __cnt   = 0; // log2 of i
    std::size_t   i     = 1; // pow of 2

    do { // wn is the unit root
        auto wn = FFT_base::root_table()[__cnt++];
        if constexpr (_Is_IFFT) wn = std::conj(wn);

        for (std::size_t j = 0 ; j != __len ; j += (i << 1)) {
            auto w = FFT_base::complex {1.0, 0.0};
            for (std::size_t k = 0 ; k != i ; (void)++k, w *= wn) {
                auto &__lhs = __cpx[j + k];
                auto &__rhs = __cpx[j + k + i];
                auto  __tmp = __rhs * w;
                __rhs = __lhs - __tmp;
                __lhs = __lhs + __tmp;
            }
        }
    } while((i <<= 1) != __len);
}

} // namespace int2048_helper

void FFT_base::FFT(complex * __cpx, std::size_t __len)
noexcept { return int2048_helper::__FFT_impl <false> (__cpx, __len); }

void FFT_base::IFFT(complex *__cpx, std::size_t __len)
noexcept { return int2048_helper::__FFT_impl <true> (__cpx, __len); }


} // namespace dark

