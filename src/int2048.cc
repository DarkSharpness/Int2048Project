#pragma once
#include "int2048.h"

#include <ranges>

namespace dark {


/* Parse the integer. */
std::istream &operator >> (std::istream &__is, int2048 &__dst) {
    __is >> __dst.buffer;
    __dst.parse(__dst.buffer);
    return __is;
}

/* Print the integer. */
std::ostream &operator << (std::ostream &__os, const int2048 &__src) {
    if (__src.is_zero()) return __os << '0';
    __src.buffer.clear();
    __src.safe_print(__src.buffer);
    return __os << __src.buffer;
}

/* Parse an integer from a string. */
void int2048::parse(std::string_view __view) {
    data.clear(); /* Pre-clear the old data. */

    auto __beg = __view.begin();
    auto __end = __view.end();

    if (*__beg == '-') { sign = true; ++__beg; }
    else sign = false;

    /* Clear all leading 0. */
    while (*__beg == '0') {
        if (++__beg == __end) return void (sign = false);
    }

    /* Calculate the length of the data. */
    const std::size_t __len = (__end - __beg - 1) / Base_Length;
    data.reserve(__len + 1);
    for (std::size_t i = 0 ; i != __len ; ++i) {
        data.push_back(parse_fold <0> (__end));
        __end -= Base_Length;
    }

    _Word_Type __last = 0;
    while(__beg != __end) { __last = __last * 10 + parse_char(*__beg++); }
    data.push_back(__last);
}


/* Print an non-zero integer. */
void int2048::safe_print(std::string &__str) const {
    using namespace std::ranges;
    using std::__detail::__to_chars_len;
    using std::__detail::__to_chars_10_impl;

    /* Init and work out the sign bit. */
    [&]() -> void {
        const std::size_t __Max_Length =
            __str.size() + sign + data.size() * Base_Length;
        __str.reserve(__Max_Length);

        if (this->is_negative()) __str.push_back('-');
    } ();

    /* Work out the highest part of the data. */
    [&]() -> void {
        const _Word_Type __high = data.back();
        const std::size_t __len = __to_chars_len(__high);
        __str.resize(__str.size() + __len);
        __to_chars_10_impl(__str.end().base() - __len, __len, __high);
    } ();

    /* Work out the lower parts of the data.  */
    [&]() -> void {
        const std::size_t __Remainder = data.size() - 1;
        if (__builtin_expect(__Remainder == 0, 0)) return;

        const std::size_t __Exact_Length =
            __str.size() + __Remainder * Base_Length;
        __str.resize(__Exact_Length);

        char *__ptr = __str.end().base();
        for (auto __cur : views::take(data, __Remainder)) {
            for (std::size_t i = 0 ; i < Base_Length ; ++i) {
                *(--__ptr) = make_char(__cur % 10);
                __cur /= 10;
            }
        }
    } ();
}

/* Convert the integer to std::string. */
std::string int2048::to_string() const {
    this->buffer.clear();
    this->print(this->buffer);
    return std::move(this->buffer);
}


/**
 * @brief Return 
 * @param __ptr Output pointer.
 * @param __beg Begin of the range.
 * @param __end End   of the range.
 * @return Whether there is a carry.
 */
bool int2048::increment
    (_Iterator __ptr,_CIterator __beg,_CIterator __end) noexcept {
    while(__beg != __end) {
        auto __carry = *(__beg++) + 1;
        if (__builtin_expect(__carry < Base, 1)) {
            return *__ptr = __carry, false;
        } else {
            *__ptr++ = __carry - Base;
        }
    } return true;
}


/**
 * @brief Internal handle of decrement in abs by 1.
 * @param src The source integer.
 * @note Make sure that this->data.size() == src.size().
 * This integer cannot be zero!
 */
bool int2048::decrement(_Iterator __ptr,_CIterator __beg,_CIterator __end) noexcept {
    while(__beg != __end) {
        auto __carry = *(__beg++) - 1;
        if (__builtin_expect(__carry < Base, 1)) {
            return !(*__ptr = __carry);
        } else {
            *__ptr++ = __carry + Base;
        }
    } __builtin_unreachable();
}


void int2048::arith_add(const _Container &lhs, const _Container &rhs) {
    auto __l = lhs.begin();
    auto __r = rhs.begin();
    auto __end = lhs.end();
    _Word_Type __carry = 0;

    while (__l != __end) {
        auto __sum = *__l + *__r + __carry;
        if (__sum < Base) {
            __carry = 0;
            data.push_back(__sum);
        } else {
            __carry = 1;
            data.push_back(__sum - Base);
        }
        ++__l; ++__r;
    }

    while (__r != rhs.end()) {
        auto __sum = *__r + __carry;
        if (__builtin_expect(__sum < Base, 1)) {
            __carry = 0;
            data.push_back(__sum);
        } else {
            __carry = 1;
            data.push_back(__sum - Base);
        }
        ++__r;
    }
    while (__r != rhs.end()) {

    }



    if (__carry) data.push_back(1);
}

void int2048::arith_sub(const _Container &lhs, const _Container &rhs) {
    
}

/* Normal == comparation. */
bool operator == (const int2048 &lhs, const int2048 &rhs) noexcept
{ return lhs.sign == rhs.sign && lhs.data == rhs.data; }

/** Three-way comparison implement. */
std::strong_ordering operator <=> (const int2048 &lhs, const int2048 &rhs) noexcept {
    if (lhs.sign != rhs.sign) return rhs.sign <=> lhs.sign;
    if (lhs.data.size() != rhs.data.size()) {
        return lhs.sign ?
            lhs.data.size() <=> rhs.data.size() :
            rhs.data.size() <=> lhs.data.size();
    }
    /* Due to data stored in reverse ordering. */
    for (auto __l = lhs.data.rbegin(), __r = rhs.data.rbegin(),
            __end = lhs.data.rend(); __l != __end; (void)++__l, ++__r) {
        auto __cmp = *__l <=> *__r;
        if (__cmp != 0) return lhs.sign ? (0 <=> __cmp) : __cmp;
    }
    return std::strong_ordering::equal;
}


/* Add the integer by 1. */
int2048 &int2048::operator ++(void) {
    if (this->is_negative()) {
        this->abs_decrement();
        sign = this->is_non_zero();
    } else {
        this->abs_increment();
    }
    return *this;
}


/* Sub the integer by 1. */
int2048 &int2048::operator --(void) {
    if (this->is_negative()) {
        this->abs_increment();
    } else {
        if (bool(sign = this->is_zero())) {
            this->data.push_back(1);
        } else {
            this->abs_decrement();
        }
    }
    return *this;
}


int2048 int2048::operator ++(int) {
    int2048 __ret { this->data.size() + 1, nullptr };
    __ret.data.resize(this->data.size());
    if (this->is_negative()) {
        __ret.try_popout(
            decrement(__ret.begin(), this->begin(), this->end())
        );
        __ret.sign = this->is_non_zero();
    } else {
        __ret.try_append(
            increment(__ret.begin(), this->begin(), this->end())
        );
        __ret.sign = false;
    }
    this->swap(__ret);
    return __ret;
}


int2048 int2048::operator --(int) {
    int2048 __ret { this->data.size() + 1, nullptr };
    __ret.data.resize(this->data.size());
    if (this->is_negative()) {
        __ret.try_append(
            increment(__ret.begin(), this->begin(), this->end())
        );
        __ret.sign = true;
    } else {
        if (bool(__ret.sign = this->is_zero())) {
            __ret.data.push_back(1);
        } else {
            __ret.try_popout(
                decrement(__ret.begin(), this->begin(), this->end())
            );
        }
        
    }
    this->swap(__ret);
    return __ret;
}


int2048 operator + (const int2048 &lhs, const int2048 &rhs) {
    if (!lhs) return rhs;
    if (!rhs) return lhs;

    int2048 __ret;
    if (lhs.sign == rhs.sign) {
        __ret.sign = lhs.sign;
        __ret.data.reserve(int2048::get_add_length(lhs.data, rhs.data));
        __ret.arith_add(lhs.data, rhs.data);
    } else {
        auto [__len,__bit] = int2048::get_sub_length(lhs.data, rhs.data);
        __ret.data.reserve(__len);
        __ret.sign = __bit ^ lhs.sign;
        if (__bit)  __ret.arith_sub(rhs.data, lhs.data);
        else        __ret.arith_sub(lhs.data, rhs.data);
    }

    return __ret;
}

// int2048 operator + (int2048 &&lhs, int2048 &&rhs) {
//     if (!lhs) return std::move(rhs);
//     if (!rhs) return std::move(lhs);
//     return 0;
// }

// int2048 operator + (const int2048 &lhs, int2048 &&rhs) { return std::move(rhs += lhs); }
// int2048 operator + (int2048 &&lhs, const int2048 &rhs) { return std::move(lhs += rhs); }

// int2048 operator - (const int2048 &lhs, int2048 &&rhs) { return std::move(rhs -= lhs); }
// int2048 operator - (int2048 &&lhs, const int2048 &rhs) { return std::move(lhs -= rhs); }


}