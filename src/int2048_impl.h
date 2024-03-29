#pragma once
#include "int2048.h"


/* Implementation of all operators of int2048. */
namespace dark {

/* Return whether the number is zero. */
bool int2048::operator !(void) const noexcept { return this->is_non_zero(); }


/* Return view to this number. */
int2048_view int2048::operator + (void) & noexcept { return  int2048_view(*this); }
/* Return view to this number with a reversed sign. */
int2048_view int2048::operator - (void) & noexcept { return -int2048_view(*this); }


/* Return this number. */
int2048 int2048::operator + (void) && noexcept { return std::move(*this); }
/* Return negative value of this number. */
int2048 int2048::operator - (void) && noexcept { return std::move(this->negate()); }


int2048 int2048::operator ++ (int) & {
    int2048 __ret = *this; this->operator ++ (); return __ret;
}

int2048 int2048::operator -- (int) & {
    int2048 __ret = *this; this->operator -- (); return __ret;
}

int2048 &int2048::operator ++ (void) & {
    if (this->sign) {
        this->abs_decrement();
        this->sign = this->is_non_zero();
    } else {
        this->abs_increment();
    }
    return *this;
}

int2048 &int2048::operator -- (void) & {
    if (this->sign) {
        this->abs_increment();
    } else {
        if (__builtin_expect(this->is_non_zero(),true)) {
            this->abs_decrement();
        } else { /* 0 - 1 = -1 */
            this->sign = true;
            this->data.reserve(Init_Length);
            this->data.resize(1);
            this->data[0] = 1;
        }
    }
    return *this;
}

int2048 int2048::operator ++ (void) && { return std::move(this->operator ++ ()); }
int2048 int2048::operator -- (void) && { return std::move(this->operator -- ()); }


int2048 operator + (int2048_view lhs, int2048_view rhs) {
    if (lhs.is_zero()) return int2048{rhs};
    if (rhs.is_zero()) return int2048{lhs};

    int2048 __ret {};
    if (lhs.sign == rhs.sign) {
        __ret.sign = lhs.sign;
        if (lhs.size() < rhs.size()) std::swap(lhs,rhs);
        __ret.data.init_capacity(lhs.size() + 1);
        __ret.data.revacancy(1); /* resize(size() - 1) */

        const auto __carry =
            int2048::add(__ret.begin(), lhs.to_unsigned(), rhs.to_unsigned());
        if (__carry) __ret.data.safe_push(__carry);
    } else { // lhs.sign != rhs.sign
        auto __cmp = lhs.size() <=> rhs.size();
        if (__cmp != 0) {
            if (__cmp < 0) std::swap(lhs,rhs);
            __ret.sign = lhs.sign;
            __ret.data.init_capacity(lhs.size());

            const auto __tail =
                int2048::sub(__ret.begin(), lhs.to_unsigned(), rhs.to_unsigned());
            __ret.data.resize(__tail);
        } else {
            const auto [__diff,__cmp] =
                int2048::cmp(lhs.to_unsigned(), rhs.to_unsigned());
            if (__cmp == 0) return __ret; /* lhs == rhs */
            if (__cmp < 0) std::swap(lhs,rhs);

            lhs.resize(__diff);
            rhs.resize(__diff);

            __ret.sign = lhs.sign;
            __ret.data.init_capacity(__diff);

            const auto __tail =
                int2048::sub(__ret.begin(), lhs.to_unsigned(), rhs.to_unsigned());
            __ret.data.resize(__tail);
        }
    }
    return __ret;
}

int2048 &operator += (int2048 &lhs, int2048_view rhs) {
    if (rhs.is_zero()) return lhs;
    if (lhs.is_zero()) return lhs = rhs;

    if (lhs.sign == rhs.sign) {
        if (lhs.size() < rhs.size()) {
            lhs.data.reserve(rhs.size() + 1);
            const auto __carry =
                int2048::add(lhs.begin(), rhs.to_unsigned(), uint2048_view {lhs});
            if (__carry) lhs.data.push_back(__carry);
        } else {
            const auto __carry =
                int2048::add(lhs.begin(), uint2048_view {lhs}, rhs.to_unsigned());
            if (__carry) lhs.data.safe_push(__carry);
        }
    } else { // lhs.sign != rhs.sign
        auto __cmp = lhs.size() <=> rhs.size();
        if (__cmp < 0) {
            lhs.sign = rhs.sign;
            lhs.data.reserve(rhs.size());
            const auto __tail =
                int2048::sub(lhs.begin(), rhs.to_unsigned(), uint2048_view {lhs});
            lhs.data.resize(__tail);
        } else if (__cmp > 0) {
            const auto __tail =
                int2048::sub(lhs.begin(), uint2048_view {lhs}, rhs.to_unsigned());
            lhs.data.resize(__tail);
        } else { // cmp == 0
            const auto [__diff,__cmp] =
                int2048::cmp(uint2048_view {lhs}, rhs.to_unsigned());

            if (__cmp == 0) return lhs.reset();

            lhs.data.resize(__diff);
            rhs.resize(__diff);

            const auto __tail =
                __cmp < 0 ? /* Whether lhs is less than rhs.  */
                   ((void)(lhs.sign = !lhs.sign),
                    int2048::sub(lhs.begin(), rhs.to_unsigned(), uint2048_view {lhs})) :
                    int2048::sub(lhs.begin(), uint2048_view {lhs}, rhs.to_unsigned())  ;

            lhs.data.resize(__tail);
        }
    } return lhs;
}

int2048 &operator += (int2048 &lhs, int2048 &&rhs) {
    /* We use the one with larger buffer. */
    if (lhs.data.capacity() < rhs.data.capacity()) lhs.swap(rhs);
    return lhs += int2048_view {rhs};
}

int2048 operator + (int2048_view lhs, int2048 &&rhs) { return std::move(rhs += lhs); }
int2048 operator + (int2048 &&lhs, int2048_view rhs) { return std::move(lhs += rhs); }
int2048 operator + (int2048 &&lhs, int2048 &&rhs) { return std::move(lhs += std::move(rhs)); }


int2048 &operator -= (int2048 &lhs, int2048_view rhs) { return lhs += rhs.negate(); }
int2048 &operator -= (int2048 &lhs, int2048 &&rhs) { return lhs += std::move(rhs.negate()); }
int2048 operator - (int2048_view lhs, int2048_view rhs) { return lhs + rhs.negate(); }
int2048 operator - (int2048_view lhs, int2048 &&rhs) { return std::move(rhs.negate() += lhs); }
int2048 operator - (int2048 &&lhs, int2048_view rhs) { return std::move(lhs += rhs.negate());  }
int2048 operator - (int2048 &&lhs, int2048 &&rhs) { return std::move(lhs += std::move(rhs.negate())); }


int2048 operator * (int2048_view lhs, int2048_view rhs) {
    int2048 __ret {};
    if (lhs.is_zero() || rhs.is_zero()) return __ret;
    __ret.sign = lhs.sign ^ rhs.sign;
    __ret.data.init_capacity(lhs.size() + rhs.size());
    __ret.data.resize(int2048::mul(__ret.begin(), lhs.to_unsigned(), rhs.to_unsigned()));
    return __ret;
}

int2048 &operator *= (int2048 &lhs, int2048_view rhs) {
    if (lhs.is_zero() || rhs.is_zero()) return lhs.reset();
    lhs.sign ^= rhs.sign;

    /* A simple test of whether brute force is enabled. */
    constexpr auto __use_brute_force = [](std::size_t __l, std::size_t __r) {
        return (__l < int2048_base::Max_Brute_Mul_Length) |
               (__r < int2048_base::Max_Brute_Mul_Length);
    };

    /**
     * @brief This may avoid the invalidation of rhs caused by self growing
     * If lhs == rhs, then if we reallocate for lhs, the rhs will be invalidated.
     * So, if lhs may not have enough capacity, we just perform normal multiplication.
     * 
     * Also, if we choose to use brute force multiplication, we need to ensure
     * that the input range will not overlap with the output range.
     */
    if (lhs.data.capacity() < lhs.size() + rhs.size()
     || __use_brute_force(lhs.size(), rhs.size())) {
        auto __temp = std::move(lhs.data);
        auto __view = uint2048_view {__temp.begin(), __temp.end()};
        lhs.data.init_capacity(__view.size() + rhs.size());
        lhs.data.resize(int2048::mul(lhs.begin(), __view, rhs.to_unsigned()));
    } else { /* Enough capcaity, so use the space of lhs as buffer. */
        lhs.data.resize(int2048::mul(lhs.begin(), uint2048_view {lhs}, rhs.to_unsigned()));
    } return lhs;
}

int2048 &operator *= (int2048 &lhs, int2048 && rhs) {
    /* We use the one with larger buffer. */
    if (lhs.data.capacity() < rhs.data.capacity()) lhs.swap(rhs);
    return lhs *= int2048_view {rhs};
}

int2048 operator * (int2048_view lhs, int2048 &&rhs) { return std::move(rhs *= lhs); }
int2048 operator * (int2048 &&lhs, int2048_view rhs) { return std::move(lhs *= rhs); }
int2048 operator * (int2048 &&lhs, int2048 &&rhs) { return std::move(lhs *= std::move(rhs)); }


} // namespace dark



/* Implementation of public member functions. */
namespace dark {


/* Explicitly construct from a view. */
int2048::int2048(int2048_view src) : data(src._beg,src._end), sign(src.sign) {}
/* Explicitly assign from a view. */
int2048 &int2048::operator = (int2048_view src) {
    if (this->begin() == src.begin()) {
        this->data.resize(src.size());
    } else {
        this->data.assign(src._beg,src._end);
    }
    this->sign = src.sign;
    return *this;
}

/* Explicitly construct from a string. */
int2048::int2048(std::string_view __str) { this->parse(__str); }
/* Explicitly assign from a string. */
int2048 &int2048::operator = (std::string_view __str) {
    this->parse(__str); return *this;
}


/* Set the sign of the number and return this. */
int2048 int2048::set_sign(bool __flag) && noexcept {
    return std::move(this->set_sign(__flag));
}
/* Set the sign of the number and return reference to this. */
int2048 &int2048::set_sign(bool __flag) & noexcept {
    this->sign = __flag & this->is_non_zero(); return *this;
}

/* Return $-this$ */
int2048 int2048::negate(void) && noexcept { return std::move(this->negate()); }
/* Negate this and return reference to this. */
int2048 &int2048::negate(void) & noexcept {
    this->sign ^= this->is_non_zero(); return *this;
}

/* Swap the content of 2 integers. */
void int2048::swap(int2048 &__other) noexcept {
    std::swap(this->data, __other.data);
    std::swap(this->sign, __other.sign);
}

/* Return $sign(this) * (abs(this) + 1)$  */
int2048 int2048::abs_increment() && { return std::move(this->abs_increment()); }


/**
 * @brief Add the absolute value of this number by 1
 * @return Reference to this number.
 */
int2048 &int2048::abs_increment() & {
    bool __carry = int2048::inc(data.begin(),uint2048_view {*this});
    if (__builtin_expect(__carry,false)) {
        this->data.safe_push(1);
    } return *this;
}

/* Return $sign(this) * (abs(this) - 1)$  */
int2048 int2048::abs_decrement() && noexcept { return std::move(this->abs_decrement()); }

/**
 * @brief Sub the absolute value of this number by 1
 * @return Reference to this number.
 * @note If the absolute value of this number is 0,
 * the behavior is undefined!
 */
int2048 &int2048::abs_decrement() & noexcept {
    bool __vacancy = int2048::dec(data.begin(),uint2048_view {*this});
    if (__builtin_expect(__vacancy,false)) {
        this->data.pop_back();
        sign &= this->is_non_zero(); /* If 0, sign = false. */
    } return *this;
}

/* Convert this number to std::string. */
std::string int2048::to_string() const {
    std::string __ret;
    int2048_view{*this}.to_string(__ret);
    return __ret;
}

/* Append this number to the given buffer. */
void int2048::to_string(std::string &__buf) const {
    return int2048_view{*this}.to_string(__buf);
}

/* Read a number from the given input stream. */
void int2048::read(std::istream &__is) {
    __is >> this->buffer;
    this->parse(this->buffer);
}

/* Parse a number from the given string. */
void int2048::parse(std::string_view __view) {
    const char *__beg = __view.begin();
    const char *__end = __view.end();

    if (*__beg != '-') this->sign = false;
    else      ++__beg, this->sign = true;

    /* Filter out all leading 0s. */
    while(*__beg == '0') if (++__beg == __end) return (void)this->reset();

    /* First, clear the data and reserve the space. */
    this->data.clear();
    std::size_t _Length =
        (__end - __beg + (Base_Length - 1)) / Base_Length;
    this->data.reserve(_Length);

    /* Then, parse the string. */
    while(--_Length) {
        this->data.push_back(parse_fold <0> (__end));
        __end -= Base_Length;
    }

    /* Import the helper function. */
    using int2048_helper::parse_char;
    _Word_Type __tmp = 0;
    while(__beg != __end) __tmp = __tmp * 10 + parse_char(*__beg++);
    this->data.push_back(__tmp);
}

/* Print the number to the given output stream. */
void int2048::print(std::ostream &__os) const {
    this->buffer.clear();
    int2048_view{*this}.to_string(this->buffer);
    __os << this->buffer;
}

/* Read a number from the given input stream. */
std::istream &operator >> (std::istream &__is, int2048 &src) {
    src.read(__is); return __is;
}

/**
 * @return Return the number of digits of this number.
 * (a.k.a. $log10(%this) + 1$)
 * @note When this is 0, it will return 1.
 */
std::size_t int2048::digits() const noexcept { return int2048_view {*this}.digits(); }

} // namespace dark



