# Basic structure

My design of int2048 is rather complex.

## vector

Yes. A vector implemented by myself. It may sounds crazy, but it is not. I just want to make it more flexible and more efficient. I don't want to use `std::vector` because it is too safe. Yes, too safe! Sometimes unnecessary checks are performed which may harm the performance. So here comes my custom vector!

Note that many functions are different from those in `std::vector`.

```C++
/**
 * @brief A simple yet fast vector implementation
 * for int2048 to use.
 */
struct vector {
  public:
    using iterator          = _Tp *;
    using const_iterator    = const _Tp *;

    vector() noexcept;
    ~vector() noexcept;
    vector (const _Tp *__beg, const _Tp *__end);

    /* Reserve __n space for the vector. */
    vector(std::size_t __n);
    vector(const vector &rhs);
    vector(vector &&rhs) noexcept;
    vector &operator = (const vector &rhs);
    vector &operator = (vector &&rhs) noexcept;

    /**
     * @brief Init the capacity of the vector.
     * @param __n The capacity of the vector.
     */
    void init_capacity(std::size_t __n);
    /**
     * @brief Force to set the capacity of the vector.
     * @param __n The new capacity of the vector.
     * @note __n should be no less than the size of the vector.
     */
    void set_capacity(std::size_t __n);
    /* Fill the back vector with zero and resize */
    void fill_size(std::size_t __n) noexcept;
    /* Push back a new element without check. */
    _Tp &push_back(const _Tp &__val) noexcept;

    _Tp &safe_push(const _Tp &__val) noexcept;

    /* Assign the vector with check. */
    void assign(const _Tp *__beg, const _Tp *__end) noexcept;

    /* Pop back a new element without check. */
    _Tp &pop_back() noexcept;
    /* Double the capacity of the vector. */
    void double_size();
    /* Clear the vector without check. */
    void clear() noexcept;
    /* Resize the vector without check. */
    void resize(std::size_t __n) noexcept;
    /* Resize the vector without check. */
    void resize(_Tp *__tail) noexcept;
    /* Resize the vector without check. */
    void revacancy(size_t __n) noexcept;
    /* Reserve the vector safely. */
    void reserve(std::size_t __n) noexcept;
    /* Shrink the vector. */
    void shrink_to_fit();

    void swap(vector &rhs) noexcept;

    _Tp &operator[](std::size_t __n) noexcept;
    const _Tp &operator[](std::size_t __n) const noexcept;

    std::size_t size()     const noexcept;
    std::size_t capacity() const noexcept;
    std::size_t vacancy () const noexcept;
    bool empty() const noexcept;

    iterator begin() noexcept;
    iterator end()   noexcept;

    const_iterator begin() const noexcept;
    const_iterator end()   const noexcept;

    const_iterator cbegin() const noexcept;
    const_iterator cend()   const noexcept;

    /* Fast testing whether equal. */
    friend bool operator == (const vector &__lhs, const vector &__rhs) noexcept;
};

```

## int2048_base

`int2048_base` is the base class that controls all behaviors of the big integers.

It is the core of int2048. Details of it as follows:

```C++
struct int2048_base {
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
    using _Container = int2048_helper::vector;

    /* Base of one word. (User defineable.) */
    inline static constexpr _Word_Type  Base = static_cast <_Word_Type> (1e9);
    /* Base length in decimal. (User defineable) */
    inline static constexpr std::size_t Base_Length = 9;
    /* Init the array with the least size. */
    inline static constexpr std::size_t Init_Sizeof = 64;
    /* Init the array with this minimum size. */
    inline static constexpr std::size_t Init_Length = Init_Sizeof / sizeof(_Word_Type);
    /* Maximum indexs a builtin-in word_type may takes. */
    inline static constexpr std::size_t Word_Length =
        std::numeric_limits <_Word_Type>::digits10 / Base_Length + 1;

  protected:
    /* Constexpr pow function. */
    constexpr static _Word_Type fast_pow(_Word_Type __x, _Word_Type __y) noexcept;

    /* Map a integer to a char. */
    constexpr static char make_char(_Word_Type __val) noexcept;
    /* Map a char into a integer. */
    constexpr static _Word_Type parse_char(char __ch) noexcept;

  protected:
    using _Iterator = typename _Container::iterator;

    static char *to_string(char *, uint2048_view) noexcept;

    using inc_t = bool;
    using cpy_t = _Iterator;
    using cmp_t = struct {
        std::size_t length;
        std::strong_ordering cmp; // It may change to ssize_t in the future.
    };
    using add_t = bool;
    using sub_t = _Iterator;

    static inc_t inc(_Iterator, uint2048_view) noexcept;
    static inc_t dec(_Iterator, uint2048_view) noexcept;

    static cpy_t cpy(_Iterator, uint2048_view) noexcept;

    static cmp_t cmp(uint2048_view,uint2048_view) noexcept;
    static add_t add(_Iterator, uint2048_view, uint2048_view) noexcept;
    static sub_t sub(_Iterator, uint2048_view, uint2048_view) noexcept;

    static void mul(_Iterator, uint2048_view, uint2048_view) noexcept;
    static void div(_Iterator, uint2048_view, uint2048_view) noexcept;
};

```


## int2048_view

`int2048_view` and `uint2048_view` are kinds of view (just like what `std::string_view` does) to an unsigned integer. They only hold 2 pointers in essence. Specially, a int2048_view will additional keep the sign of the integer.

This is designed for better abstraction and performance. When we are operating between 2 integers, we just care about the range that represent it and the sign of it (iff signed integer). Also, consider `operator -` and `operator +`. We may want to return kind of `reference` to the integer in a different sign possibly. This is why we need a view to the integer. (I know that experienced users can reduce most of these problems with move-semantics and the ::negate() member functions, but I still want to keep it simple and easy to use, so here comes the views.)

Here are some of the public (non-)member functions of `int2048_view` and `uint2048_view`:

```C++
/**
 * @brief View of an unsigned integer.
 * It will not copy the data of the integer,
 * but provide a view of the integer.
 */
struct uint2048_view : int2048_base {
  public:
    uint2048_view() = default;
    uint2048_view(const uint2048 &) noexcept;

    explicit uint2048_view(const int2048 &) noexcept;
    explicit uint2048_view(int2048_view) noexcept;
    
    explicit operator std::string() const;
    std::string to_string() const;
    void to_string(std::string &) const;

    std::size_t digits() const noexcept;

    _Iterator begin() const noexcept { return _beg; }
    _Iterator end()   const noexcept { return _end; }

    friend bool operator == (uint2048_view, uint2048_view) noexcept;
    friend std::strong_ordering operator <=> (uint2048_view, uint2048_view) noexcept;

    bool is_zero()          const noexcept;
    bool is_non_zero()      const noexcept;
};

/**
 * @brief View of a signed integer.
 * It will not copy the data of the integer,
 * but provide a view of the integer.
 */
struct int2048_view : int2048_base {
  public:
    int2048_view() = default;
    int2048_view(const int2048 &) noexcept;

    explicit int2048_view(const uint2048 &) noexcept;
    explicit int2048_view(uint2048_view) noexcept;
    explicit int2048_view(uint2048_view,bool) noexcept;

    explicit operator std::string() const;
    std::string to_string() const;
    void to_string(std::string &) const;

    friend std::ostream &operator << (std::ostream &, int2048_view);

    _Iterator begin() const noexcept { return _beg; }
    _Iterator end()   const noexcept { return _end; }

    int2048_view operator + (void) const noexcept;
    int2048_view operator - (void) const noexcept;

    int2048_view set_sign(bool) noexcept;
    int2048_view negate() noexcept;
    std::size_t digits() const noexcept;

    friend int2048 operator + (int2048_view, int2048_view);
    friend int2048 operator - (int2048_view, int2048_view);
    friend int2048 operator * (int2048_view, int2048_view);

    friend int2048 &operator += (int2048 &lhs, int2048_view rhs);

    friend bool operator == (int2048_view, int2048_view) noexcept;
    friend std::strong_ordering operator <=> (int2048_view, int2048_view) noexcept;

    uint2048_view to_unsigned() const noexcept;

    bool is_zero()          const noexcept;
    bool is_non_zero()      const noexcept;

    bool is_negative()      const noexcept;
    bool is_non_negative()  const noexcept;

    bool is_positive()      const noexcept;
    bool is_non_positive()  const noexcept;
};
```


## int2048

Most of those core behaviors have been defined in `int2048_base`. So `int2048` is just a wrapper of `int2048_view` with some additional member functions. All arithmetic operations are resolved in `int2048_base`. We just perform corresponding operations on the views and references based on the input.

What's special, we take right value into consideration. Since we may generate a lot of temporary expression, we reload the operator for rvalue to avoid unnecessary copies.


```C++
struct int2048 : int2048_base {
    static_assert(fast_pow(10, Base_Length) == Base, "Wrongly implemented!");
  public:
    /* Constructors and assignements. */

    int2048() noexcept(noexcept(_Container()));
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

    /* Construct from a single word.  */
    template <typename _Tp>
    requires std::same_as <_Tp, _Word_Type>
    int2048(_Tp __val);

    /* Construct from a single word with given sign. */
    int2048(_Word_Type __val, bool __sign);

    /* Construct from any signed integer. */
    int2048(std::intmax_t __val);

    /* Assign from a single word. */
    template <typename _Tp>
    requires std::same_as <_Tp, _Word_Type>
    int2048 &operator = (_Tp __val);

    /* Assign from any signed integer. */
    int2048 &operator = (std::intmax_t __val);

    template <typename _Tp>
    requires std::is_signed_v <_Tp>
    explicit operator _Tp() const noexcept;

    template <typename _Tp>
    requires std::is_unsigned_v <_Tp>
    explicit operator _Tp() const noexcept;

    explicit operator bool() const noexcept;

    explicit operator std::string() const;

    /* Reset this integer to 0. */
    int2048 &reset() & noexcept;

    /* Return 0. */
    int2048 reset() && noexcept;

    bool is_zero()          const noexcept;
    bool is_non_zero()      const noexcept;
    bool is_negative()      const noexcept;
    bool is_non_negative()  const noexcept;
    bool is_positive()      const noexcept;
    bool is_non_positive()  const noexcept;

};

```

# Caution

TODO: