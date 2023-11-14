#pragma once

#include "utility.h"
#include <limits>
#include <complex>
#include <cstring>
#include <cstdint>
#include <iostream>

/* Some declarations. */
namespace dark {

struct int2048;
struct int2048_view;
struct uint2048;
struct uint2048_view;
struct int2048_base;

} // namespace dark

namespace dark {

struct FFT_base {
  protected:
    /* Maximum bit length of FFT. */
    inline static constexpr std::size_t FFT_Max = 20;

    using complex       = std::complex <double>;
    using _Word_Type    = std::uintmax_t;
    /* Container for FFT operation. */
    using FFT_t         = int2048_helper::vector <complex>;
    /* Table reference for FFT operation. */
    using table_t       = struct _FFT_Table {
        complex *   data; // Unit root table.
        std::size_t bits; // Log2 of table size.
    };

    /* FFT Zipping times. */
    inline static constexpr std::size_t FFT_Zip     = 2;
    /* FFT Base Length. */
    inline static constexpr std::size_t FFT_BaseLen = 4;
    /* FFT Base Word. */
    inline static constexpr _Word_Type  FFT_Base    = int2048_helper::__pow(10U, FFT_BaseLen);

    static void FFT(complex *, std::size_t) noexcept;
    [[__gnu__::__always_inline__]]
    static inline void merge_FFT(complex *, complex *) noexcept;
    [[__gnu__::__always_inline__]]
    static inline void final_FFT(complex *, complex *) noexcept;
    [[__gnu__::__always_inline__]]
    static inline void FFT_pass(FFT_t &) noexcept;

    static table_t make_table(std::size_t);
};

/**
 * @brief Base class for int2048.
 * 
 */
struct int2048_base : protected FFT_base {
  public:
    /**
     * A common buffer for iostream operations only.
     * Users may perform anything they want on it.
     * It just works as a flexible space for better performance.
     * 
     * Here is one example of how users may use this buffer:
     * 1. Call ::to_string(int2048_base::buffer) of an integer
     * to print out the same integer for multiple times.
     * 
     */
    inline static std::string buffer {};
  protected:

    using _Word_Type = std::uintmax_t;
    using _Container = int2048_helper::vector <_Word_Type>;

    /* Base of one word. */
    inline static constexpr _Word_Type  Base = int2048_helper::__pow(FFT_Base, FFT_Zip);
    /* Base length in decimal. */
    inline static constexpr std::size_t Base_Length = FFT_BaseLen * FFT_Zip;
    /* Init the array with the least size. */
    inline static constexpr std::size_t Init_Sizeof = 64;
    /* Init the array with this minimum size. */
    inline static constexpr std::size_t Init_Length = Init_Sizeof / sizeof(_Word_Type);
    /* Maximum indexs a builtin-in Word_Type may takes. */
    inline static constexpr std::size_t Word_Length =
        std::numeric_limits <_Word_Type>::digits10 / Base_Length + 1;
    /* Maximum length of brute force multiplication. */
    inline static constexpr std::size_t Max_Brute_Mul_Length = 256;
    /* Maximum length of brute force division and mod. */
    inline static constexpr std::size_t Max_Brute_Div_Length = 1;

  protected:
    using _Iterator = typename _Container::iterator;
    using _CIterator= typename _Container::const_iterator; 

    static char *to_string(char *, uint2048_view) noexcept;

    using inc_t = bool;
    using dec_t = bool;
    using cpy_t = _Iterator;
    using cmp_t = struct _Cmp_Type {
        using cmp_result_t = std::strong_ordering;
        std::size_t length; /* Length of the different. */
        cmp_result_t cmp;   /* Compare result.          */
    };
    using add_t = bool;
    using sub_t = _Iterator;
    using mul_t = _Iterator;
    using div_t = _Iterator;
    using mod_t = _Iterator;
    using inv_t = _Iterator;

    static inc_t inc(_Iterator, uint2048_view) noexcept;
    static dec_t dec(_Iterator, uint2048_view) noexcept;
    static cpy_t cpy(_Iterator, uint2048_view) noexcept;
    static cmp_t cmp(uint2048_view,uint2048_view) noexcept;
    static add_t add(_Iterator, uint2048_view, uint2048_view) noexcept;
    static sub_t sub(_Iterator, uint2048_view, uint2048_view) noexcept;
    static mul_t mul(_Iterator, uint2048_view, uint2048_view);
    static div_t div(_Iterator, uint2048_view, uint2048_view);
    static mod_t mod(_Iterator, uint2048_view, uint2048_view);

    static inv_t inv(_Iterator, uint2048_view) noexcept;

  protected:

    static bool use_brute_mul(uint2048_view&, uint2048_view&) noexcept;
    static bool use_brute_div(uint2048_view&, uint2048_view&) noexcept;
    static bool use_brute_mod(uint2048_view&, uint2048_view&);

    static mul_t brute_mul(_Iterator, uint2048_view, uint2048_view) noexcept;
    static div_t brute_div(_Iterator, uint2048_view, uint2048_view);
    static mod_t brute_mod(_Iterator, uint2048_view, uint2048_view) noexcept;

    static FFT_t make_FFT(uint2048_view, uint2048_view);

    static uint2048_view try_div(_Iterator, uint2048_view, uint2048_view) noexcept;

    static div_t adjust_div(_Iterator,_Iterator,uint2048_view,uint2048_view,uint2048_view) noexcept;
    static mod_t adjust_mod(_Iterator,_Iterator,uint2048_view,uint2048_view,uint2048_view) noexcept;

  protected:

    /* Unfold template of parsing a string. */
    template <std::size_t _Beg = 0>
    [[nodiscard, __gnu__::__always_inline__]]
    static _Word_Type parse_fold(const char *__str) noexcept {
        using namespace int2048_helper;
        if constexpr (_Beg == Base_Length) { return 0; }
        else {
            return parse_fold <_Beg + 1> (__str) +
                parse_char(__str[-_Beg - 1]) * __pow(10U, _Beg);
        }
    }
    [[nodiscard, __gnu__::__always_inline__]]
    static _Word_Type parse_string(const char *__str) noexcept { return parse_fold(__str); }
    [[nodiscard, __gnu__::__always_inline__]]
    static _Word_Type narrow_down(_CIterator, std::size_t) noexcept;
    [[nodiscard, __gnu__::__always_inline__]]
    static _Iterator init_value(_Iterator, _Word_Type) noexcept;

  public:
    /* Return the maximum possible length of the integer in decimal. */
    static consteval std::size_t max_digits() noexcept { return FFT_BaseLen << (FFT_Max - 1); }
};

/**
 * @brief View of an unsigned integer.
 * It will not copy the data of the integer,
 * but provide a view of the integer.
 */
struct uint2048_view : int2048_base {
  protected:
    friend class int2048_base;
    friend class int2048_view;
    friend class int2048;
    friend class uint2048;

    using _Base_Type = int2048_base;
    using _Base_Type::_Word_Type;
    using _Base_Type::_Container;
    using _Iterator = typename _Container::const_iterator;
    using _Base_Type::to_string;

    _Iterator _beg;
    _Iterator _end;


    uint2048_view(_Iterator __beg, _Iterator __end)
    noexcept : _beg(__beg), _end(__end) {}

    void resize(std::size_t __size) noexcept { _end = _beg + __size; }
    std::size_t size() const noexcept { return _end - _beg; }

  public:
    uint2048_view() = default;

    explicit uint2048_view(const int2048 &) noexcept;
    // explicit uint2048_view(const int2048 &&) = delete;
    uint2048_view(const uint2048 &) noexcept;
    // uint2048_view(const uint2048 &&) = delete;

    explicit uint2048_view(int2048_view) noexcept;

    explicit operator std::string() const;

  public:

    friend int2048 &operator *= (int2048 &lhs, int2048_view rhs);

    std::string to_string() const;
    void to_string(std::string &) const;
    std::size_t digits() const noexcept;

    _Iterator begin() const noexcept { return _beg; }
    _Iterator end()   const noexcept { return _end; }

    friend bool operator == (uint2048_view, uint2048_view) noexcept;
    friend std::strong_ordering operator <=> (uint2048_view, uint2048_view) noexcept;

    [[nodiscard]]
    bool is_zero()          const noexcept { return _beg == _end; }
    [[nodiscard]]
    bool is_non_zero()      const noexcept { return !this->is_zero(); }
};

/**
 * @brief View of a signed integer.
 * It will not copy the data of the integer,
 * but provide a view of the integer.
 */
struct int2048_view : int2048_base {
  protected:
    friend class uint2048_view;
    friend class int2048;
    friend class uint2048;

    using _Base_Type = int2048_base;
    using _Base_Type::_Word_Type;
    using _Base_Type::_Container;
    using _Iterator = typename _Container::const_iterator;
    using _Base_Type::to_string;

    _Iterator _beg;
    _Iterator _end;
    bool sign;

    int2048_view(_Iterator __beg, _Iterator __end, bool __sign = false)
    noexcept : _beg(__beg), _end(__end), sign(__sign) {}

    void resize(std::size_t __size) noexcept { _end = _beg + __size; }
    std::size_t size() const noexcept { return _end - _beg; }

  public:
    int2048_view() = default;

    int2048_view(const int2048 &) noexcept;
    // int2048_view(const int2048 &&) = delete;
    explicit int2048_view(const uint2048 &) noexcept;
    // explicit int2048_view(const uint2048 &&) = delete;

    explicit int2048_view(uint2048_view) noexcept;
    explicit int2048_view(uint2048_view,bool) noexcept;

    explicit operator std::string() const;

  public:
    _Iterator begin() const noexcept { return _beg; }
    _Iterator end()   const noexcept { return _end; }

    int2048_view operator + (void) const noexcept;
    int2048_view operator - (void) const noexcept;

    std::string to_string() const;
    void to_string(std::string &) const;

    int2048_view set_sign(bool) noexcept;
    int2048_view negate() noexcept;
    std::size_t digits() const noexcept;

    friend int2048 operator + (int2048_view, int2048_view);
    friend int2048 operator - (int2048_view, int2048_view);
    friend int2048 operator * (int2048_view, int2048_view);

    friend int2048 &operator += (int2048 &lhs, int2048_view rhs);
    friend int2048 &operator *= (int2048 &lhs, int2048_view rhs);

    friend bool operator == (int2048_view, int2048_view) noexcept;
    friend std::strong_ordering operator <=> (int2048_view, int2048_view) noexcept;

  public:

    uint2048_view to_unsigned() const noexcept { return uint2048_view {_beg, _end}; }

    [[nodiscard]]
    bool is_zero()          const noexcept { return _beg == _end; }
    [[nodiscard]]
    bool is_non_zero()      const noexcept { return !this->is_zero(); }

    bool is_negative()      const noexcept { return  sign; }
    [[nodiscard]]
    bool is_non_negative()  const noexcept { return !sign; }

    friend std::ostream &operator << (std::ostream &, int2048_view);

    [[nodiscard]]
    bool is_positive()      const noexcept { return !is_negative() && !this->is_zero(); }
    [[nodiscard]]
    bool is_non_positive()  const noexcept { return  is_negative() ||  this->is_zero(); }
};

struct int2048 : int2048_base {
  protected:
    // using namespace int2048_helper;

    // Some static assertions.
    static_assert(int2048_helper::__pow(10U, Base_Length) == Base,  "Wrongly implemented!");
    static_assert(int2048_helper::__pow(FFT_Base,FFT_Zip) == Base,  "Wrongly implemented!");
    static_assert(Init_Length >= Word_Length                     ,  "Wrongly implemented!");
    static_assert(Base * Base < -1ULL / (Max_Brute_Mul_Length + 1)   ,  "Wrongly implemented!");

    friend class int2048_view;
    friend class uint2048_view;

    using _Base_Type = int2048_base;
    using _Base_Type::_Word_Type;
    using _Base_Type::_Container;
    using _Iterator  = typename _Container::iterator;

    _Container  data; /* Data of the integer.   */
    bool        sign; /* Sign of the integer.   */

    /* Return the size of the data. */
    std::size_t size() const noexcept { return data.size(); }

    _Iterator begin() noexcept { return data.begin(); }
    _Iterator end()   noexcept { return data.end(); }

  public:
    /* Constructors and assignements. */

    constexpr int2048() noexcept(noexcept(_Container())) : data{}, sign{false} {}
    ~int2048() = default;

    int2048(int2048 &&) = default;
    int2048(const int2048 &) = default;
    int2048 &operator = (const int2048 &)   = default;
    int2048 &operator = (int2048 &&)        = default;

    explicit int2048(int2048_view);
    int2048 &operator = (int2048_view);

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
    int2048 operator ++ (int)&& = delete; /* Completely useless. */
    int2048 operator -- (int)&& = delete; /* Completely useless. */

    int2048 &operator ++ (void)&;
    int2048 &operator -- (void)&;
    int2048 operator ++ (void)&&; /* Sometimes this works better. */
    int2048 operator -- (void)&&; /* Sometimes this works better. */

    friend int2048 &operator += (int2048 &, int2048_view);
    friend int2048 &operator += (int2048 &, int2048 &&);

    friend int2048 operator + (int2048_view, int2048_view);
    friend int2048 operator + (int2048_view, int2048 &&);
    friend int2048 operator + (int2048 &&, int2048_view);
    friend int2048 operator + (int2048 &&, int2048 &&);

    friend int2048 &operator -= (int2048 &, int2048_view);
    friend int2048 &operator -= (int2048 &, int2048 &&);

    friend int2048 operator - (int2048_view, int2048_view);
    friend int2048 operator - (int2048_view, int2048 &&);
    friend int2048 operator - (int2048 &&, int2048_view);
    friend int2048 operator - (int2048 &&, int2048 &&);

    friend int2048 &operator *= (int2048 &, int2048_view);
    friend int2048 &operator *= (int2048 &, int2048 &&);

    friend int2048 operator * (int2048_view, int2048_view);
    friend int2048 operator * (int2048_view, int2048 &&);
    friend int2048 operator * (int2048 &&, int2048_view);
    friend int2048 operator * (int2048 &&, int2048 &&);

    int2048 &operator /= (const int2048 &);
    friend int2048 operator / (int2048, const int2048 &);

    int2048 &operator %= (const int2048 &);
    friend int2048 operator % (int2048, const int2048 &);

    friend std::istream &operator >> (std::istream &, int2048 &);

  public:
    void swap(int2048 &) noexcept;

    int2048 &abs_increment()&;
    int2048 abs_increment()&&;
    int2048 &abs_decrement()& noexcept;
    int2048 abs_decrement()&& noexcept;

    void parse(std::string_view);
    std::string to_string() const;
    void to_string(std::string &) const;

    int2048 &set_sign(bool) & noexcept;
    int2048 set_sign(bool) && noexcept;

    int2048 &negate() & noexcept;
    int2048 negate() && noexcept;

    std::size_t digits() const noexcept;

    void read(std::istream & = std::cin);
    void print(std::ostream & = std::cout) const;

  public:
    /**
     * ----------------------------------
     * 
     * Some implementation in .h file.
     * 
     * ----------------------------------
     */

    /* Construct from a single word.  */
    template <std::same_as <_Word_Type> _Tp>
    int2048(_Tp __val) : data{} , sign(false) {
        if (__val == 0) return;
        data.init_capacity(Init_Length);
        data.resize(this->init_value(data.begin(), __val));
    }

    /* Construct from a single word with given sign. */
    explicit int2048(_Word_Type __val, bool __sign) : data{} , sign(__sign) {
        if (__val == 0) { sign = false; return; }
        data.init_capacity(Init_Length);
        data.resize(this->init_value(data.begin(), __val));
    }

    /* Construct from any signed integer. */
    int2048(std::intmax_t __val)
        : int2048 {
            __val < 0 ? -static_cast <_Word_Type> (__val) : __val, __val < 0
        } {}

    /* Assign from a single word. */
    template <std::same_as <_Word_Type> _Tp>
    int2048 &operator = (_Tp __val) {
        this->reset();
        if (__builtin_expect(__val != 0, true)) {
            data.reserve(Init_Length);
            data.resize(init_value(this->data.begin(), __val));
        }
        return *this;
    }

    /* Assign from any signed integer. */
    int2048 &operator = (std::intmax_t __val) {
        data.clear();
        if (__builtin_expect(__val != 0, true)) {
            if (bool(sign = __val < 0)) __val = -__val;
            data.reserve(Init_Length);
            data.resize(init_value(this->data.begin(), __val));
        } else { /* If val == 0 */
            this->sign = false;
        } return *this;
    }

    template <std::signed_integral _Tp>
    explicit operator _Tp() const noexcept {
        const _Tp __ret = static_cast <std::make_unsigned_t <_Tp>> (*this);
        return sign ? -__ret : __ret;
    }

    template <typename _Tp>
    requires std::is_unsigned_v <_Tp>
    explicit operator _Tp() const noexcept { return narrow_down(data.begin(),data.size()); }

    explicit operator bool() const noexcept { return this->is_non_zero(); }

    explicit operator std::string() const { return this->to_string(); }

    /* Reset this integer to 0. */
    int2048 &reset() & noexcept { sign = 0; data.clear(); return *this; }
    /* Return 0. */
    int2048 reset() && noexcept { return std::move(this->reset()); }

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

};



} // namespace dark
