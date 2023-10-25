#pragma once

#include <memory>
#include <cstdint>
#include <cstring>

namespace dark::int2048_helper {

/**
 * @brief A simple yet fast vector implementation
 * for int2048 to use.
 */
struct vector {
  protected:
    using _Tp       = std::uintmax_t;
    using _Alloc    = std::allocator <_Tp>;

    [[no_unique_address]] _Alloc alloc;
    _Tp *head;
    _Tp *tail;
    _Tp *term;

    /* Steal the data from rhs and reset rhs. */
    void steal(vector &rhs) noexcept {
        head = rhs.head, tail = rhs.tail, term = rhs.term;
        // std::memcpy(this, &rhs, sizeof(vector));
        return rhs.reset();
    }

    /* Reset the inner pointers to null. */
    void reset() noexcept {
        head = tail = term = nullptr;
        // std::memset(this, 0, sizeof(vector));
    }

    /* Deallocate the memory inside. */
    void deallocate() noexcept {
        alloc.deallocate(head, this->capacity());
    }

  public:
    using iterator          = _Tp *;
    using const_iterator    = const _Tp *;

    vector() noexcept : head(), tail(), term() {}
    ~vector() noexcept { this->deallocate(); }

    vector (const _Tp *__beg, const _Tp *__end) : vector(__end - __beg) {
        tail = term;
        std::memcpy(head, __beg, this->size() * sizeof(_Tp));
    }

    /* Reserve __n space for the vector. */
    vector(std::size_t __n) : head(alloc.allocate(__n)), tail(head), term(head + __n) {}

    vector(const vector &rhs) : vector(rhs.size())  {
        std::memcpy(head, rhs.head, rhs.size() * sizeof(_Tp));
    }

    vector(vector &&rhs) noexcept { this->steal(rhs); }

    vector &operator = (const vector &rhs) {
        if (this != &rhs) {
            this->reserve(rhs.size());
            std::memcpy(head, rhs.head, rhs.size() * sizeof(_Tp));
        } return *this;
    }

    vector &operator = (vector &&rhs) noexcept {
        if (this != &rhs) {
            this->deallocate();
            this->steal(rhs);
        } return *this;
    }

  public:

    /**
     * @brief Init the capacity of the vector.
     * @param __n The capacity of the vector.
     */
    void init_capacity(std::size_t __n) {
        tail = head = alloc.allocate(__n);
        term = head + __n;
    }

    /**
     * @brief Force to set the capacity of the vector.
     * @param __n The new capacity of the vector.
     * @note __n should be no less than the size of the vector.
     */
    void set_capacity(std::size_t __n) {
        _Tp *const __next = alloc.allocate(__n);
        std::memcpy(__next, head, this->size() * sizeof(_Tp));
        this->deallocate();
        tail = __next + this->size();
        term = (head = __next) + __n;
    }

    /* Fill the back vector with zero and resize */
    void fill_size(std::size_t __n) {
        auto *__next = head + __n;
        std::memset(tail, 0, (__next - tail)  * sizeof(_Tp));
        tail = __next;
    }

    /* Push back a new element without check. */
    _Tp &push_back(const _Tp &__val) noexcept { return *(tail++) = __val; }

    _Tp &safe_push(const _Tp &__val) noexcept {
        if (this->vacancy() == 0) this->double_size();
        return this->push_back(__val);
    }

    /* Assign the vector with check. */
    void assign(const _Tp *__beg, const _Tp *__end) noexcept {
        const std::size_t _Length = __end - __beg;
        if (_Length > this->capacity()) {
            this->deallocate();
            this->init_capacity(_Length);
        }
        this->resize(_Length);
        std::memmove(head, __beg, _Length * sizeof(_Tp));
    }

    /* Pop back a new element without check. */
    _Tp &pop_back() noexcept { return *--tail; }

    /* Double the capacity of the vector. */
    void double_size() {
        return this->set_capacity(this->size() | !this->size());
    }

    /* Clear the vector without check. */
    void clear() noexcept { tail = head; }

    /* Resize the vector without check. */
    void resize(std::size_t __n) noexcept { tail = head + __n; }

    /* Resize the vector without check. */
    void resize(_Tp *__tail) noexcept { tail = __tail; }

    /* Resize the vector without check. */
    void revacancy(size_t __n) noexcept { tail = term - __n; }

    /* Reserve the vector safely. */
    void reserve(std::size_t __n) noexcept {
        if (__n > this->size()) this->set_capacity(__n);   
    }

    /* Shrink the vector. */
    void shrink_to_fit() {
        if (this->vacancy() != 0) this->set_capacity(this->size());
    }

    _Tp &operator[](std::size_t __n) noexcept { return head[__n]; }
    const _Tp &operator[](std::size_t __n) const noexcept { return head[__n]; }

  public:
    /* Some necessary interfaces. */

    std::size_t size()     const noexcept { return tail - head; }
    std::size_t capacity() const noexcept { return term - head; }
    std::size_t vacancy () const noexcept { return term - tail; }
    bool empty() const noexcept { return head == tail; }

    iterator begin() noexcept { return head; }
    iterator end()   noexcept { return tail; }

    const_iterator begin() const noexcept { return head; }
    const_iterator end()   const noexcept { return tail; }

    const_iterator cbegin() const noexcept { return head; }
    const_iterator cend()   const noexcept { return tail; }

    /* Fast testing whether equal. */
    friend bool operator == (const vector &__lhs, const vector &__rhs) noexcept {
        return __lhs.size() == __rhs.size() &&
            std::memcmp(__lhs.head, __rhs.head, __lhs.size() * sizeof(_Tp)) == 0;
    }
};

} // namespace dark::int2048_helper

