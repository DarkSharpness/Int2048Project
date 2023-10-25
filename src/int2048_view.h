#pragma once
#include "int2048.h"


namespace dark {

/* Construct explicitly from a big integer. */
uint2048_view::uint2048_view(const int2048 &__int)
noexcept : _beg(__int.data.begin()), _end(__int.data.end()) {}

/* Not completed... */
uint2048_view::uint2048_view(const uint2048 &) noexcept { __builtin_unreachable(); }

/* Construct explicitly from a signed view. */
uint2048_view::uint2048_view(int2048_view __int)
noexcept : _beg(__int._beg), _end(__int._end) {}

/* Convert explicitly to string. */
uint2048_view::operator std::string() const { return this->to_string(); }

std::string uint2048_view::to_string() const {
    std::string __ret;
    this->to_string(__ret);
    return __ret;
}

void uint2048_view::to_string(std::string &__buf) const {
    if (this->is_zero()) { __buf += '0'; return; }
    const std::size_t _Max_Length =
        __buf.size() + this->size() * Base_Length;

    char *__end = __buf.end().base();
    __buf.resize(_Max_Length);

    /* Update the new end. */
    __end = this->to_string(__end, *this);
    __buf.resize(__end - __buf.data());
}

/**
 * @return Return the number of digits of this number.
 * (a.k.a. $log10(%this) + 1$)
 * @note When this is 0, it will return 1.
 */
std::size_t uint2048_view::digits() const noexcept {
    if (this->is_zero()) return 1;
    return (this->size() - 1) * Base_Length
        + std::__detail::__to_chars_len(*(_end - 1));
}

bool operator == (uint2048_view lhs, uint2048_view rhs) noexcept {
    constexpr std::size_t _Sizeof = sizeof(uint2048_view::_Word_Type);
    return lhs.size() == rhs.size() &&
        std::memcmp(lhs._beg,rhs._beg,lhs.size() * _Sizeof) == 0;
}

std::strong_ordering operator <=> (uint2048_view lhs, uint2048_view rhs) noexcept {
    if (lhs.size() != rhs.size()) return lhs.size() <=> rhs.size();
    else if (lhs.size() == 0) return std::strong_ordering::equal;
    else return int2048_base::cmp(lhs,rhs).cmp;
}

} // namespace dark



namespace dark {

/* Construct implicitly from a big integer. */
int2048_view::int2048_view(const int2048 &__int)
noexcept : _beg(__int.data.begin()), _end(__int.data.end()), sign(__int.sign) {}

/* Not completed... */
int2048_view::int2048_view(const uint2048 &) noexcept { __builtin_unreachable(); }

/* Construct explicitly from an unsigned view.  */
int2048_view::int2048_view(uint2048_view __int)
noexcept : int2048_view(__int,false) {}

/* Construct explicitly from an unsigned view and a sign. */
int2048_view::int2048_view(uint2048_view __int,bool __sign)
noexcept : _beg(__int._beg), _end(__int._end), sign(__sign) {}

/* Perform nothing to the view. */
int2048_view int2048_view::operator + (void)
const noexcept { return *this; }

/* Reverse the sign of the view. */
int2048_view int2048_view::operator - (void)
const noexcept { return int2048_view {*this}.negate(); }

/* Print out the number to the given output stream. */
std::ostream &operator << (std::ostream &__os, int2048_view __int) {
    __int.buffer.clear();
    __int.to_string(__int.buffer);
    return __os << __int.buffer;
}

/* Convert explicitly to string. */
int2048_view::operator std::string() const { return this->to_string(); }

/* Set the sign of the view. */
int2048_view int2048_view::set_sign(bool __sign) noexcept {
    this->sign = __sign & this->is_non_zero(); return *this;
}

/* Negate the view (equal to this->reverse(). */
int2048_view int2048_view::negate(void) noexcept {
    this->sign ^= this->is_non_zero(); return *this;
}

/* Convert the view to a number string. */
std::string int2048_view::to_string() const {
    std::string __ret;
    this->to_string(__ret);
    return __ret;
}

/* Output the view to a given string. */
void int2048_view::to_string(std::string &__buf) const {
    if (this->is_zero()) { __buf += '0'; return; }

    const std::size_t _Max_Length =
        __buf.size() + this->size() * Base_Length + this->sign;
    __buf.reserve(_Max_Length);

    if (this->sign) __buf.push_back('-');

    /* Old end as the beginning. */
    char *__end = __buf.end().base();
    __buf.resize(_Max_Length);

    /* Update the new end. */
    __end = this->to_string(__end, this->to_unsigned());
    __buf.resize(__end - __buf.data());
}

/**
 * @return Return the number of digits of this number.
 * (a.k.a. $log10(%this) + 1$)
 * @note When this is 0, it will return 1.
 */
std::size_t int2048_view::digits() const noexcept {
    if (this->is_zero()) return 1;
    return (this->size() - 1) * Base_Length
        + std::__detail::__to_chars_len(*(_end - 1));
}

/* Compare two numbers. */
bool operator == (int2048_view lhs, int2048_view rhs) noexcept {
    return lhs.sign == rhs.sign && lhs.to_unsigned() == rhs.to_unsigned();
}

/* Compare two numbers. */
std::strong_ordering operator <=> (int2048_view lhs, int2048_view rhs) noexcept {
    if (lhs.sign != rhs.sign) return rhs.sign <=> lhs.sign;
    return lhs.sign ?
        rhs.to_unsigned() <=> lhs.to_unsigned() :
        lhs.to_unsigned() <=> rhs.to_unsigned();
}

} // namespace dark
