#pragma once
#include "int2048.h"


namespace dark {

/* Construct explicitly from a big integer. */
uint2048_view::uint2048_view(const int2048 &__int)
noexcept : _beg(__int.data.begin()), _end(__int.data.end()) {}

/* Not completed... */
uint2048_view::uint2048_view(const uint2048 &__int) noexcept { throw; }

/* Construct explicitly from a signed view. */
uint2048_view::uint2048_view(int2048_view __int)
noexcept : _beg(__int._beg), _end(__int._end) {}

/* Convert explicitly to a signed view. */
uint2048_view::operator int2048_view() const noexcept {
    return int2048_view {_beg, _end};
}

/* Convert explicitly to string. */
uint2048_view::operator std::string() const {
    return this->to_string();
}

} // namespace dark



namespace dark {

/* Construct implicitly from a big integer. */
int2048_view::int2048_view(const int2048 &__int)
noexcept : _beg(__int.data.begin()), _end(__int.data.end()), sign(__int.sign) {}

/* Not completed... */
int2048_view::int2048_view(const uint2048 &) noexcept { throw ""; }

/* Construct explicitly from an unsigned view.  */
int2048_view::int2048_view(uint2048_view __int)
noexcept : int2048_view(__int,false) {}

/* Construct explicitly from an unsigned view and a sign. */
int2048_view::int2048_view(uint2048_view __int,bool __sign)
noexcept : _beg(__int._beg), _end(__int._end), sign(__sign) {}

/* Perform nothing to the view. */
int2048_view int2048_view::operator +(void)
const noexcept { return *this; }

/* Reverse the sign of the view. */
int2048_view int2048_view::operator -(void)
const noexcept { return int2048_view {*this}.reverse(); }

/* Explicitly convert to a unsigned view. */
int2048_view::operator uint2048_view() const noexcept {
    return uint2048_view {_beg, _end};
}

/* Convert explicitly to string. */
int2048_view::operator std::string() const {
    return this->to_string();
}

/* Set the sign of the view. */
int2048_view int2048_view::set_sign(bool __sign) noexcept {
    this->sign = __sign; return *this;
}

/* Reverse the sign of the view. */
int2048_view int2048_view::reverse(void) noexcept {
    this->sign ^= true;  return *this;
}


} // namespace dark
