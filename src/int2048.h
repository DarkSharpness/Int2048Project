#pragma once

#include <vector>
#include <limits>
#include <cstring>
#include <cstdint>
#include <iostream>

namespace dark {


struct int2048_base {
  protected:
    using _Word_Type = std::uintmax_t;
    using _Container = std::vector <_Word_Type>;

    /* Base of one word. */
    inline static constexpr _Word_Type  Base        = static_cast <_Word_Type> (1e9);

    /* Base length in decimal. */
    inline static constexpr std::size_t Base_Length = 9;

    /* Init the array with the least size. */
    inline static constexpr std::size_t Init_Sizeof = 64;

    /* Init the array with this minimum size. */
    inline static constexpr std::size_t Init_Length = Init_Sizeof / sizeof(_Word_Type);

    /* Maximum indexs a builtin-in word_type may takes. */
    inline static constexpr std::size_t Word_Length =
        std::numeric_limits <_Word_Type>::digits10 / Base_Length + 1;

  protected:
    /* Consteval pow function. */
    consteval static _Word_Type cexp_pow(_Word_Type __x, _Word_Type __y) {
        _Word_Type __ret = 1;
        while(__y--) __ret *= __x;
        return __ret;
    }

    /* Map a integer to a char. */
    constexpr static char make_char(_Word_Type __val) { return __val | '0'; }

    /* Map a char into a integer. */
    constexpr static _Word_Type parse_char(char __ch) { return __ch & 0xf; }
};


struct int2048 : int2048_base {
  public:
    static_assert(cexp_pow(10, Base_Length) == Base, "Wrongly implemented!");

  protected:

    static consteval std::size_t s_hash(const char *__str) {
        while (*__str == '0') ++__str;
    }

    /* Resize/reserve factor for a better buffering performance. */
    static std::size_t factor(std::size_t __n) noexcept {
        return __n + ((__n >> 1) | 1);
    }

    static std::size_t get_add_length
        (const _Container &__lhs, const _Container &__rhs) noexcept {
        return std::max(__lhs.size(), __rhs.size());
    }

    static ssize_t get_sub_length
        (const _Container &__lhs, const _Container &__rhs) noexcept {
        if (__lhs.size() != __rhs.size()) return get_add_length(__lhs, __rhs);
        for (auto __l = __lhs.rbegin(), __r = __rhs.rbegin(), 
                  __end = __lhs.rend() ; __l != __end ; (void)++__l, ++__r) {
            if (*__l != *__r) {
                ssize_t __ret = __end - __l;
                return *__l < *__r ? -__ret : __ret;
            }
        } return 0;
    }

    using _Base_Type = int2048_base;
    using _Base_Type::_Word_Type;
    using _Base_Type::_Container;

    using _Base_Type::Base;
    using _Base_Type::Init_Length;
    using _Base_Type::Base_Length;
    using _Base_Type::Word_Length;

    using _Base_Type::cexp_pow;
    using _Base_Type::make_char;
    using _Base_Type::parse_char;

    _Container  data; /* Data of the integer.   */
    bool        sign; /* Sign of the integer.   */

    /* Common buffer for in and out. */
    inline static std::string buffer {};

    /* Unfold template of init from an __val. */
    template <std::size_t _Beg>
    void init_fold(std::uintmax_t __val) {
        static_assert (_Beg <= Word_Length, "Too large!");
        if constexpr (_Beg == Word_Length) {
            for (std::size_t __i = 0; __i < _Beg; ++__i) {
                data.push_back(__val % this->Base);
                __val /= Base;
            }
        } else {
            if (__val < cexp_pow(Base, _Beg)) {
                /* Compiler should unroll this loop. */
                for (std::size_t __i = 0; __i < _Beg; ++__i) {
                    data.push_back(__val % this->Base);
                    __val /= Base;
                }
            } else return init_fold <_Beg + 1> (__val);
        }
    }

    /* Unfold template of parsing string to int. */
    template <std::size_t _Beg>
    _Word_Type parse_fold(const char *data) const {
        if constexpr (_Beg == Base_Length) { return 0; }
        else {
            return parse_fold <_Beg + 1> (data) +
                parse_char(data[-_Beg - 1]) * cexp_pow(10, _Beg);
        }
    }

    /* Unfold template of narrowing to a builtin-type. */
    template <std::size_t _Beg>
    _Word_Type narrow_fold() const noexcept {
        if constexpr (_Beg == Word_Length) return 0;
        else {
            if (data.size() == _Beg) return 0;
            return data[_Beg] *cexp_pow(Base, _Beg) + narrow_fold <_Beg + 1> ();
        }
    }

    void safe_print(std::string &) const;

    void increment(const _Container &);
    void decrement(const _Container &) noexcept;

    void arith_add(const _Container &, const _Container &);
    void arith_sub(const _Container &, const _Container &);

    void arith_mul(const _Container &, const _Container &);
    void arith_div(const _Container &, const _Container &);

  public:

    int2048() noexcept(noexcept(_Container())) : data{}, sign{false} {}
    ~int2048() = default;

    int2048(int2048 &&) = default;
    int2048(const int2048 &) = default;
    int2048 &operator = (const int2048 &)   = default;
    int2048 &operator = (int2048 &&)        = default;

    /* Construct from a single word.  */
    template <typename _Tp>
    requires std::same_as <_Tp, _Word_Type>
    int2048(_Tp __val) : data{} , sign(false) {
        if (__val == 0) { sign = false; return; }
        data.reserve(std::max(Init_Length, Word_Length));
        init_fold <1> (__val);
    }

    /* Construct from a single word with given sign. */
    int2048(_Word_Type __val, bool __sign) : int2048{__val} { sign = __sign; }

    /* Construct from any signed integer. */
    int2048(std::intmax_t __val)
        : int2048 {
            __val < 0 ? -static_cast <_Word_Type> (__val) : __val, __val < 0
        } {}

    /* Assign from a single word. */
    template <typename _Tp>
    requires std::same_as <_Tp, std::uintmax_t>
    int2048 &operator = (_Tp __val) {
        this->reset();
        if (__val != 0) init_fold <1> (__val);
        return *this;
    }

    /* Assign from any signed integer. */
    int2048 &operator = (std::intmax_t __val) {
        return *this = ((sign = __val < 0) ?
            -static_cast <_Word_Type> (__val) : __val); 
    }

    /* Construct from an string. */
    explicit int2048(std::string_view __src) { this->parse(__src); }

  public:
    /* Return true iff this integer is zero. */
    bool operator !(void) const noexcept { return this->is_zero(); }

    friend int2048 operator - (const int2048 &__src)
    noexcept { auto __tmp = __src; __tmp.reverse(); return __tmp; }
    friend int2048 operator - (int2048 &&__src)
    noexcept { return std::move(__src.reverse()); }

    friend const int2048 &operator + (const int2048 &__src) noexcept { return __src; }
    friend int2048 operator + (int2048 &&__src) noexcept  { return std::move(__src); }

    int2048 operator ++ (int);
    int2048 operator -- (int);

    int2048 &operator ++ (void);
    int2048 &operator -- (void);

    friend int2048 &operator += (int2048 &, const int2048 &);
    friend int2048 &operator += (int2048 &, int2048 &&);
    friend int2048 operator + (const int2048 &, const int2048 &);
    friend int2048 operator + (int2048 &&, int2048 &&);

    friend int2048 &operator -= (int2048 &, const int2048 &);
    friend int2048 &operator -= (int2048 &, int2048 &&);
    friend int2048 operator - (const int2048 &, const int2048 &);
    friend int2048 operator - (int2048 &&, int2048 &&);

    int2048 &operator *= (const int2048 &);
    friend int2048 operator * (int2048, const int2048 &);

    int2048 &operator /= (const int2048 &);
    friend int2048 operator / (int2048, const int2048 &);

    int2048 &operator %= (const int2048 &);
    friend int2048 operator % (int2048, const int2048 &);

    friend std::istream &operator >> (std::istream &, int2048 &);
    friend std::ostream &operator << (std::ostream &, const int2048 &);

    friend bool operator == (const int2048 &, const int2048 &) noexcept;
    friend std::strong_ordering operator <=> (const int2048 &, const int2048 &) noexcept;

    explicit operator bool() const noexcept { return this->is_non_zero(); }

    explicit operator std::string() const { return this->to_string(); }

    template <typename _Tp>
    requires std::is_signed_v <_Tp>
    explicit operator _Tp() const noexcept {
        const _Tp __ret = static_cast <_Tp > (narrow_fold <0> ());
        return sign ? -__ret : __ret;
    }

    template <typename _Tp>
    requires std::is_unsigned_v <_Tp>
    explicit operator _Tp() const noexcept { return narrow_fold <0> (); }

  public:

    void swap (int2048 &__other) noexcept {
        std::swap(this->data, __other.data);
        std::swap(this->sign, __other.sign);
    }

    /* Increase the absolute value by 1 and return reference to self. */
    int2048 &abs_increment() { this->increment(this->data); return *this; }
    /* Decrease the absolute value by 1 and return reference to self. */
    int2048 &abs_decrement() /* Decrement won't allocate space, thus noexcept. */
                    noexcept { this->decrement(this->data); return *this; }

    void parse(std::string_view);
    std::string to_string() const;
    /**
     * @brief Append the integer to the string.
     * @param __str The string to be appended.
     */
    void print(std::string &__str) const {
        this->is_zero() ? __str.push_back('0') : this->safe_print(__str);
    }

    /**
     * @brief Set the sign of the integer.
     * @return Reference to this integer.
     */
    int2048 &set_sign(bool __sign) & noexcept { sign = __sign & bool(*this); return *this; }

    /**
     * @brief Set the sign of the integer.
     * @return Reference to this integer.
     */
    int2048 set_sign(bool __sign) && noexcept { return std::move(this->set_sign(__sign)) ; }

    /**
     * @brief Reverse the sign of the integer.
     * @return Reference to this integer.
     */
    int2048 &reverse() & noexcept { sign ^= bool(*this); return *this; }

    int2048 reverse() && noexcept { return std::move(this->reverse()); }

    bool is_zero()          const noexcept { return data.empty(); }
    [[nodiscard]]
    bool is_non_zero()      const noexcept { return !this->is_zero(); }

    bool is_negative()      const noexcept { return  sign; }
    [[nodiscard]]
    bool is_non_negative()  const noexcept { return !sign; }

    [[nodiscard]]
    bool is_positive()      const noexcept { return !is_negative() && !this->is_zero(); }
    [[nodiscard]]
    bool is_non_positive()  const noexcept { return  is_negative() ||  this->is_zero(); }

    /* Reset this integer to 0. */
    void reset() noexcept { sign = 0; data.clear(); }
};



} // namespace dark
