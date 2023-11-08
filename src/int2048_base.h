#pragma once

#include "int2048.h"
#include <bit>

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
auto int2048_base::sub(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs)
noexcept -> sub_t {
    bool __carry = 0;
    auto __lhs = lhs.begin();
    for(const auto __cur : rhs) {
        const _Word_Type __sum = *(__lhs++) - __cur - __carry;
        *__ptr++ = (__carry = __sum > Base - 1) ? __sum + Base : __sum;
    }

    if (__carry) {
        const auto __dist = lhs.end() - __lhs;    
        // Example: 1000 - 999 = 0001 = 1
        if (__dist == 1 && *__lhs == 1) {
            while (__ptr[-1] == 0) --__ptr; // Remove the leading 0.
        } else { // Example 10000 - 999 = 09001 = 9001
            __ptr += __dist - dec(__ptr, {__lhs,lhs.end()});
        } return __ptr;
    } else {
        if (__lhs != lhs.end()) { // Example 1999 - 999 = 1000
            __ptr = cpy(__ptr, {__lhs,lhs.end()});
        } else { // Example: 2000 - 1999 = 0001 = 1
            while (__ptr[-1] == 0) --__ptr; // Remove the leading 0.
        } return __ptr;
    }
}

/**
 * @brief Mul lhs and rhs to __ptr.
 * @param __ptr Output range.
 * @return Iterator to the tail of the result.
 */
auto int2048_base::mul(_Iterator __ptr,uint2048_view lhs,uint2048_view rhs)
-> mul_t {
    if (use_brute_mul(lhs,rhs)) return brute_mul(__ptr,lhs,rhs);

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

    if (__ptr[-1] == 0) --__ptr; // Remove the leading 0.
    return __ptr;
}

} // namespace dark

namespace dark {

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
    // __tmp.dat /= (1ull << __shift)
    return __tmp.dat;
}


} // namespace int2048_helper

/**
 * @return Whether lhs and rhs should be multiplied by brute force.
 * @note After this function, lhs will be no smaller than rhs in size.
 */
bool int2048_base::use_brute_mul(uint2048_view &lhs, uint2048_view &rhs) 
noexcept {
    if (lhs.size() < rhs.size()) std::swap(lhs,rhs);
    return lhs.size() < Max_Brute_Length;
}

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
 * @note lhs should be no smaller than rhs in size.
 */
auto int2048_base::make_fft(
    uint2048_view lhs,
    uint2048_view rhs,
    const std::size_t _Max_Length) -> fft_t {
    using namespace int2048_helper;

    const auto _Bit_Length  = 2 + __log2(_Max_Length - 1);
    const auto _Length      = std::size_t {1} << _Bit_Length;

    fft_t __fft; __fft.init_capacity(_Length);

    auto __lhs = lhs.begin();

    const int _LShift = 1 + __log2(lhs.size());
    const int _Rshift = _Bit_Length + 1 - _LShift;

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
void int2048_base::swap_rev(complex *__cpx,const idx_t *__rev, std::size_t __len)
noexcept {
    for (std::size_t i = 0; i < __len; ++i)
        if (i < __rev[i]) // Avoid swap twice.
            std::swap(__cpx[i], __cpx[__rev[i]]);
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


auto FFT_base::root_table() noexcept -> const complex *  {
    using cpx = std::complex <double>;
    static constexpr FFT_Table_t __table = { // cos (pi / (1 << i)) , sin(pi / (1 << i))
        cpx{ -1.0 , 0.0 }, // 0
        cpx{ 0.0  , 1.0 }, // 1
        cpx{ 0.7071067811865476 , 0.7071067811865475 }, // 2
        cpx{ 0.9238795325112867 , 0.3826834323650898 }, // 3
        cpx{ 0.9807852804032304 , 0.19509032201612825 }, // 4
        cpx{ 0.9951847266721969 , 0.0980171403295606 }, // 5
        cpx{ 0.9987954562051724 , 0.049067674327418015 }, // 6
        cpx{ 0.9996988186962042 , 0.024541228522912288 }, // 7
        cpx{ 0.9999247018391445 , 0.012271538285719925 }, // 8
        cpx{ 0.9999811752826011 , 0.006135884649154475 }, // 9
        cpx{ 0.9999952938095762 , 0.003067956762965976 }, // 10
        cpx{ 0.9999988234517019 , 0.0015339801862847655 }, // 11
        cpx{ 0.9999997058628822 , 0.0007669903187427045 }, // 12
        cpx{ 0.9999999264657179 , 0.00038349518757139556 }, // 13
        cpx{ 0.9999999816164293 , 0.0001917475973107033 }, // 14
        cpx{ 0.9999999954041073 , 9.587379909597734e-05 }, // 15
        cpx{ 0.9999999988510269 , 4.793689960306688e-05 }, // 16
        cpx{ 0.9999999997127567 , 2.396844980841822e-05 }, // 17
        cpx{ 0.9999999999281892 , 1.1984224905069705e-05 }, // 18
        cpx{ 0.9999999999820472 , 5.9921124526424275e-06 }, // 19
    };
    return __table.data();
}

} // namespace dark

