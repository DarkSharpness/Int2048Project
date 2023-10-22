#pragma once

#include <vector>
#include <limits>
#include <cstring>
#include <cstdint>
#include <iostream>

namespace dark {


struct int2048_base {
  protected:
    /* Common buffer for in and out. */
    inline static std::string buffer {};

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

struct int2048;
struct int2048_view;
struct uint2048;
struct uint2048_view;

struct uint2048_view : int2048_base {
  protected:
    friend class int2048_view;
    friend class int2048;
    friend class uint2048;

    using _Base_Type = int2048_base;
    using _Base_Type::_Word_Type;
    using _Base_Type::_Container;
    using _Iterator = typename _Container::const_iterator;

    _Iterator _beg;
    _Iterator _end;

    uint2048_view(_Iterator __beg, _Iterator __end)
    noexcept : _beg(__beg), _end(__end) {}

  public:
    uint2048_view() = default;

    explicit uint2048_view(const int2048 &) noexcept;
    explicit uint2048_view(const int2048 &&) = delete;
    uint2048_view(const uint2048 &) noexcept;
    uint2048_view(const uint2048 &&) = delete;

    explicit uint2048_view(int2048_view) noexcept;
    explicit operator int2048_view() const noexcept;
    explicit operator std::string() const { return this->to_string(); }


  public:
    std::string to_string() const;
    void to_string(std::string &) const;

    _Iterator begin() const noexcept { return _beg; }
    _Iterator end()   const noexcept { return _end; }
};


struct int2048_view : int2048_base {
  protected:
    friend class uint2048_view;
    friend class int2048;
    friend class uint2048;

    using _Base_Type = int2048_base;
    using _Base_Type::_Word_Type;
    using _Base_Type::_Container;
    using _Iterator = typename _Container::const_iterator;

    _Iterator _beg;
    _Iterator _end;
    bool sign;

    int2048_view(_Iterator __beg, _Iterator __end, bool __sign = false)
    noexcept : _beg(__beg), _end(__end), sign(__sign) {}

  public:
    int2048_view() = default;

    int2048_view(const int2048 &) noexcept;
    int2048_view(const int2048 &&) = delete;
    explicit int2048_view(const uint2048 &) noexcept;
    explicit int2048_view(const uint2048 &&) = delete;

    explicit int2048_view(uint2048_view,bool) noexcept;
    explicit int2048_view(uint2048_view) noexcept;

    explicit operator uint2048_view() const noexcept;
    explicit operator std::string() const { return this->to_string(); }

  public:
    _Iterator begin() const noexcept { return _beg; }
    _Iterator end()   const noexcept { return _end; }

    int2048_view operator + (void) const noexcept;
    int2048_view operator - (void) const noexcept;

    std::string to_string() const;
    void to_string(std::string &) const;

    int2048_view set_sign(bool) noexcept;
    int2048_view reverse() noexcept;

    friend int2048 operator + (int2048_view, int2048_view);
    friend int2048 operator - (int2048_view, int2048_view);
    friend int2048 operator * (int2048_view, int2048_view);
};


struct int2048 : int2048_base {
  public:
    static_assert(cexp_pow(10, Base_Length) == Base, "Wrongly implemented!");

  protected:
    using _Base_Type = int2048_base;
    using _Base_Type::_Word_Type;
    using _Base_Type::_Container;
    using _Iterator  = typename _Container::iterator;
    using _CIterator = typename _Container::const_iterator; 

    using _Base_Type::Base;
    using _Base_Type::Init_Length;
    using _Base_Type::Base_Length;
    using _Base_Type::Word_Length;

    using _Base_Type::cexp_pow;
    using _Base_Type::make_char;
    using _Base_Type::parse_char;

    _Container  data; /* Data of the integer.   */
    bool        sign; /* Sign of the integer.   */

    /* Init the array with a given size. */
    int2048(size_t __n,std::nullptr_t) { data.reserve(__n); }

    void safe_print(std::string &) const;

  public:
    /* Constructors and assignements. */

    int2048() noexcept(noexcept(_Container())) : data{}, sign{false} {}
    ~int2048() = default;


    int2048(int2048 &&) = default;
    int2048(const int2048 &) = default;
    int2048 &operator = (const int2048 &)   = default;
    int2048 &operator = (int2048 &&)        = default;

    explicit int2048(int2048_view);
    int2048 &operator = (int2048_view);

    /* Construct from an string. */
    explicit int2048(std::string_view);
    int2048 &operator = (std::string_view);

  public:
    /* Return true iff this integer is zero. */
    bool operator !(void) const noexcept;

    int2048_view operator + (void)& noexcept;
    int2048_view operator - (void)& noexcept;

    int2048 operator + (void)&& noexcept;
    int2048 operator - (void)&& noexcept;

    int2048 operator ++ (int)&;
    int2048 operator -- (int)&;

    int2048 &operator ++ (void)&;
    int2048 &operator -- (void)&;
    int2048 &operator ++ (void)&&;
    int2048 &operator -- (void)&&;

    friend int2048 &operator += (int2048 &, int2048_view);
    friend int2048 &operator += (int2048 &, int2048 &&);

    friend int2048 operator + (int2048_view, int2048 &&);
    friend int2048 operator + (int2048 &&, int2048_view);
    friend int2048 operator + (int2048 &&, int2048 &&);

    friend int2048 &operator -= (int2048 &, int2048_view);
    friend int2048 &operator -= (int2048 &, int2048 &&);

    friend int2048 operator - (int2048_view, int2048 &&);
    friend int2048 operator - (int2048 &&, int2048_view);
    friend int2048 operator - (int2048 &&, int2048 &&);

    friend int2048 &operator *= (int2048 &, int2048_view);
    friend int2048 &operator *= (int2048 &, int2048 &&);

    friend int2048 operator * (int2048_view, int2048 &&);
    friend int2048 operator * (int2048 &&, int2048_view);
    friend int2048 operator * (int2048 &&, int2048 &&);

    int2048 &operator /= (const int2048 &);
    friend int2048 operator / (int2048, const int2048 &);

    int2048 &operator %= (const int2048 &);
    friend int2048 operator % (int2048, const int2048 &);

    friend std::istream &operator >> (std::istream &, int2048 &);
    friend std::ostream &operator << (std::ostream &, const int2048 &);

    friend bool operator == (const int2048 &, const int2048 &) noexcept;
    friend std::strong_ordering operator <=> (const int2048 &, const int2048 &) noexcept;

  public:

    void swap (int2048 &__other) noexcept;

    int2048 &abs_increment();
    int2048 &abs_decrement() noexcept;

    void parse(std::string_view);
    std::string to_string() const;
    void to_string(std::string &) const;

    int2048 &set_sign(bool) & noexcept;
    int2048 set_sign(bool) && noexcept;
    int2048 &reverse() & noexcept;
    int2048 reverse() && noexcept;

  public:

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
    requires std::same_as <_Tp, _Word_Type>
    int2048 &operator = (_Tp __val) {
        this->reset();
        if (__val != 0) init_fold <1> (__val);
        return *this;
    }

    /* Assign from any signed integer. */
    int2048 &operator = (std::intmax_t __val) {
        data.clear();
        init_fold <1> ((sign = __val < 0) ?
            -static_cast <_Word_Type> (__val) : __val);
        return *this;
    }


    /* Implementations in .h file. */

    template <typename _Tp>
    requires std::is_signed_v <_Tp>
    explicit operator _Tp() const noexcept {
        const _Tp __ret = static_cast <_Tp > (narrow_fold <0> ());
        return sign ? -__ret : __ret;
    }

    template <typename _Tp>
    requires std::is_unsigned_v <_Tp>
    explicit operator _Tp() const noexcept { return narrow_fold <0> (); }

    explicit operator bool() const noexcept { return this->is_non_zero(); }

    explicit operator std::string() const { return this->to_string(); }

    /* Reset this integer to 0. */
    void reset() noexcept { sign = 0; data.clear(); }

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

    /* Init from an _Word_Type. */
    template <std::size_t _Beg>
    void init_fold(_Word_Type __val) {
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

    /* Parsing string to int. */
    template <std::size_t _Beg>
    _Word_Type parse_fold(const char *data) const {
        if constexpr (_Beg == Base_Length) { return 0; }
        else {
            return parse_fold <_Beg + 1> (data) +
                parse_char(data[-_Beg - 1]) * cexp_pow(10, _Beg);
        }
    }

    /* Narrowing down to a builtin-type. */
    template <std::size_t _Beg>
    _Word_Type narrow_fold() const noexcept {
        if constexpr (_Beg == Word_Length) return 0;
        else {
            if (data.size() == _Beg) return 0;
            return data[_Beg] *cexp_pow(Base, _Beg) + narrow_fold <_Beg + 1> ();
        }
    }

};



} // namespace dark
