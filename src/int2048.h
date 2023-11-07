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
    /* Maximum bit length of FFT. */
    inline static constexpr std::size_t FFT_Max = 20;

    using complex       = std::complex <double>;
    using FFT_Table_t   = std::array <complex, FFT_Max>;
    static const complex *root_table() noexcept {
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

    using _Word_Type = std::uintmax_t;

    /* FFT zip times. */
    inline static constexpr std::size_t FFT_Zip     = 2;
    /* FFT Base Word. */
    inline static constexpr _Word_Type  FFT_Base    = 1e4;

    static void  FFT(complex *, std::size_t) noexcept;
    static void IFFT(complex *, std::size_t) noexcept;
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
    inline static constexpr _Word_Type  Base = static_cast <_Word_Type> (1e8);
    /* Base length in decimal. */
    inline static constexpr std::size_t Base_Length = 8;
    /* Init the array with the least size. */
    inline static constexpr std::size_t Init_Sizeof = 64;
    /* Init the array with this minimum size. */
    inline static constexpr std::size_t Init_Length = Init_Sizeof / sizeof(_Word_Type);
    /* Maximum indexs a builtin-in word_type may takes. */
    inline static constexpr std::size_t Word_Length =
        std::numeric_limits <_Word_Type>::digits10 / Base_Length + 1;
    /* Maximum length of brute force multiplication. */
    inline static constexpr std::size_t Max_Brute_Length = 16;

  protected:

    /* Constexpr pow function. */
    static constexpr _Word_Type fast_pow(_Word_Type __x, _Word_Type __y)
    noexcept {
        _Word_Type __ret = 1;
        while(__y) {
            if (__y & 1) __ret *= __x;
            __x *= __x; __y >>= 1;
        } return __ret;
    }

    /* Map a integer to a char. */
    static constexpr char make_char(_Word_Type __val) noexcept { return __val | '0'; }

    /* Map a char into a integer. */
    static constexpr _Word_Type parse_char(char __ch) noexcept { return __ch & 0xf; }

  protected:
    using _Iterator = typename _Container::iterator;

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
    using div_t = struct _Div_Type {
        std::size_t length; /* Length of the quotient.  */
        _Container  remain; /* Remainders in container. */
    };

    /* The container for FFT operation. */
    using fft_t = int2048_helper::vector <complex>;
    using idx_t = std::uint32_t;
    using rev_t = int2048_helper::vector <idx_t>;

    static inc_t inc(_Iterator, uint2048_view) noexcept;
    static dec_t dec(_Iterator, uint2048_view) noexcept;

    static cpy_t cpy(_Iterator, uint2048_view) noexcept;

    static cmp_t cmp(uint2048_view,uint2048_view) noexcept;
    static add_t add(_Iterator, uint2048_view, uint2048_view) noexcept;
    static sub_t sub(_Iterator, uint2048_view, uint2048_view) noexcept;

    static mul_t mul(_Iterator, uint2048_view, uint2048_view);
    static div_t div(_Iterator, uint2048_view, uint2048_view);

  protected:

    static bool use_brute_mul(uint2048_view&, uint2048_view&) noexcept;
    static bool use_brute_div(uint2048_view&, uint2048_view&) noexcept;

    static mul_t brute_mul(_Iterator, uint2048_view, uint2048_view) noexcept;
    static div_t brute_div(_Iterator, uint2048_view, uint2048_view) noexcept;

    static fft_t make_fft(uint2048_view,uint2048_view, const std::size_t);
    static rev_t make_rev(std::size_t);

    static void swap_rev(complex *,const idx_t *, std::size_t) noexcept;

  public:
    // static constexpr std::size_t max_size() noexcept {}
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
    static_assert(fast_pow(10 , Base_Length) == Base,   "Wrongly implemented!");
    static_assert(fast_pow(FFT_Base,FFT_Zip) == Base,   "Wrongly implemented!");

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
    template <typename _Tp>
    requires std::same_as <_Tp, _Word_Type>
    int2048(_Tp __val) : data{} , sign(false) {
        if (__val == 0) { sign = false; return; }
        data.init_capacity(std::max(Init_Length, Word_Length));
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
        if (__builtin_expect(__val != 0, true)) init_fold <1> (__val);
        return *this;
    }

    /* Assign from any signed integer. */
    int2048 &operator = (std::intmax_t __val) {
        data.clear();
        if (__builtin_expect(__val != 0, true)) {
            init_fold <1> ((sign = __val < 0) ?
                -static_cast <_Word_Type> (__val) : __val);  
        } else { /* If val == 0 */
            sign = false;
        } return *this;
    }

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

  protected:

    /* Init from an _Word_Type. */
    template <std::size_t _Beg>
    void init_fold(_Word_Type __val) noexcept {
        static_assert (_Beg <= Word_Length, "Too large!");
        if constexpr (_Beg == Word_Length) {
            for (std::size_t __i = 0; __i < _Beg; ++__i) {
                data.push_back(__val % this->Base);
                __val /= Base;
            }
        } else {
            if (__val < fast_pow(Base, _Beg)) {
                /* Compiler should unroll this loop. */
                for (std::size_t __i = 0; __i < _Beg; ++__i) {
                    data.push_back(__val % this->Base);
                    __val /= Base;
                }
            } else return init_fold <_Beg + 1> (__val);
        }
    }

    /* Narrowing down to a builtin-type. */
    template <std::size_t _Beg>
    _Word_Type narrow_fold() const noexcept {
        if constexpr (_Beg == Word_Length) return 0;
        else {
            if (data.size() == _Beg) return 0;
            return data[_Beg] *fast_pow(Base, _Beg) + narrow_fold <_Beg + 1> ();
        }
    }

    /* Parsing string to int. */
    template <std::size_t _Beg>
    static _Word_Type parse_fold(const char *__str) noexcept {
        if constexpr (_Beg == Base_Length) { return 0; }
        else {
            return parse_fold <_Beg + 1> (__str) +
                parse_char(__str[-_Beg - 1]) * fast_pow(10, _Beg);
        }
    }

};



} // namespace dark
