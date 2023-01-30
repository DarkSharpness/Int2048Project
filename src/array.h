#ifndef _DARK_ARRAY_TCC_
#define _DARK_ARRAY_TCC_

#include <memory>
#include <initializer_list>
#include <stddef.h>
#include <string.h>

namespace dark::number {

/**
 * @brief Standard array for value type.
 * This requires that the value_t must be combination of basic
 * types , which have no special destruction function or move
 * construction function , and value_t will be initlized by 0.
 * .(For example,int,double,float.) 
 * 
 * @tparam value_t Default is double.
 */
template <class value_t = double>
class Array : std::allocator <value_t> {
  private:
    value_t *head; // Head of data.
    value_t *tail; // Tail of data.
    value_t *term; // Terminal of the space.

    /* Copy elements from src.(memcpy)
       Space should be allocated beforehand. */
    static void mcopy(void *dst,const void *src,size_t __n) 
    { memcpy (dst,src,sizeof(value_t) * __n); }
    /* Copy elements from src.(memmove)
       Space should be allocated beforehand. */
    static void mmove(void *dst,const void *src,size_t __n) 
    { memmove(dst,src,sizeof(value_t) * __n); }
    /* Clear given the data by 0. */
    static void mset(void *dst,size_t length) 
    { memset(dst,0,sizeof(value_t) * length); }


    /* Alloc __n space in all. */
    void realloc(size_t __n) {
        value_t *temp = this->allocate(__n);
        mcopy(temp,head,size());
        this->~Array();
        tail += (temp - head);
        term = (head = temp) + __n;   
        return;
    }


  public:
    /* Free the memory. */
    ~Array() noexcept { this->deallocate(head,capacity()); }

    /* Initialize as empty(). */
    Array() { head = tail = term = nullptr; }

    /* Initialize to given length.(Data uninitalized) */
    Array(size_t length) {
        head = tail = this->allocate(length);
        term = head + length;
    }

    /* Initialize to given length.(Data initalized with 0) */
    Array(size_t length,std::nullptr_t) {
        head = tail = this->allocate(length);
        term = head + length;
        mset(head,length);
    }

    /* Initialize from an consecutive memory. */
    template <class Iterator>
    Array(Iterator first,size_t __n) {
        tail = (head = this->allocate(__n)) + __n;
        term = head + __n;
        mcopy(head,(const void *)&(*first),__n);
    }

    /* Initialize with a list. */
    Array(std::initializer_list <value_t> L) : Array(L.begin(),L.size()) {}

    /* Initialize from another %array. */
    Array(const Array &src) : Array(src.head,src.size()) {}

    /* Initialize from another %array. */
    Array(Array &&src) noexcept : 
        head(src.head),tail(src.tail),term(src.term) {
        src.head = src.tail = src.term = nullptr;
    }

    /* Initialize with a bias of __n elements.
       Equal to src << __n */
    Array(intmax_t __n,const Array &src) {
        size_t __size = __n + src.size();
        if(intmax_t(__size) > 0) {
            term = tail = (head = this->allocate(__size)) + __size;
            if(__n > 0) {
                mcopy(head + __n,src.head,src.size());
                mset(head,__n);
            } else { /* -__n >= 0 */
                mcopy(head,src.head - __n,__size);
            }
        } else {
            head = tail = term = nullptr;
        }
    }

    /* Copy data from rhs. */
    Array &operator = (const Array &rhs) {
        if(this != &rhs) {
            if(capacity() < rhs.size()) *this = Array(rhs);
            else {
                tail = head + rhs.size();
                mcopy(head,rhs.head,rhs.size());
            }
        }
        return *this;
    }

    /* Move data from rhs. */
    Array &operator = (Array &&rhs) noexcept {
        this->~Array();
        head = rhs.head;
        tail = rhs.tail;
        term = rhs.term;
        rhs.head = rhs.tail = rhs.term = nullptr;
        return *this;
    }

    /* Test whether the %array is empty. */
    bool empty() const noexcept { return head == tail; }
    /* Return the number of elements in the %array now. */
    size_t size() const noexcept { return tail - head; }
    /* Return the number of elements % the array can hold now. */
    size_t vacancy() const noexcept { return term - tail; }
    /* Return the maximum size of elements the %array can reach now. */
    size_t capacity() const noexcept { return term - head; }

    /* Reserve space before hand. */
    void reserve(size_t __n) {
        if(__n > capacity()) realloc(__n);
    }

    /* Resize to __n without initialization. */
    void resize(size_t __n) {
        reserve(__n);
        tail = head + __n;
    }

    /* Resize to __n with initialization as 0. */
    void resize(size_t __n,std::nullptr_t) {
        if(__n > size()) {
            reserve(__n);
            mset(tail,__n - size());
        }
        tail = head + __n;
    }

    /* Special operation. */
    Array &operator <<= (intmax_t __n) {
        if(__n > 0) expand_front(__n);
        else if(__n < 0) {
            size_t __size = size() + __n;
            if(intmax_t(__size) > 0) {
                mmove(head,head - __n,__size);
                tail = head + __size;
            } else { clear(); }
        }
        return *this;
    }
    /* Special operation. */
    Array &operator >>= (intmax_t __n) { return *this <<= -__n; }

    /* Special operation. */
    friend Array operator << (const Array &lhs,intmax_t __n) 
    { return Array(__n,lhs); }
    /* Special operation. */
    friend Array operator >> (const Array &lhs,intmax_t __n) 
    { return Array(-__n,lhs); }

    /* Special operation. */
    friend Array operator << (Array &&lhs,intmax_t __n) 
    { return std::move(lhs <<= __n); }
    /* Special operation. */
    friend Array operator >> (Array &&lhs,intmax_t __n)
    { return std::move(lhs <<= -__n); }

    /* Expand back and fill with 0. */
    void expand_back(size_t __n) {
        reserve(size() + __n);
        mset(tail,__n);
        tail += __n;
    }

    /* Expand front and fill with 0 */
    void expand_front(size_t __n) {
        if(__n > vacancy()) {
            value_t *temp = this->allocate(size() + __n);
            mcopy(temp + __n,head,size());
            this->~Array();
            term = tail += (temp - head) + __n;
            head = temp;
        } else mmove(head + __n,head,size());
        mset(head,__n);
    }

    /* Push back one element safely. */
    void push_back(value_t val) { 
        if(tail == term) realloc(size() << 1 | empty());
        insert_back(val);
    }

    /* Push_back one element dangerously.*/
    void insert_back(value_t val) noexcept { *(tail++) = val; }

    /* Pop one element. */
    void pop_back() noexcept { --tail; }

    /* Clear the %array. */
    void clear() noexcept { tail = head; }

    /* Swap 2 containers. */
    void swap(Array &rhs) noexcept {
        std::swap(head,rhs.head);
        std::swap(tail,rhs.tail);
        std::swap(term,rhs.term);
    }

    value_t &operator [](size_t __n)             noexcept { return head[__n]; }
    const value_t &operator [](size_t __n) const noexcept { return head[__n]; }


    value_t &front() noexcept { return *begin(); }
    value_t &back()  noexcept { return *(end() - 1); }
    const value_t &front() const noexcept { return *cbegin(); }
    const value_t &back()  const noexcept { return *(end() - 1); }

    using iterator       = value_t *;
    using const_iterator = const value_t *;

    iterator begin() noexcept { return head; }
    iterator end()   noexcept { return tail; }

    const_iterator begin()  const noexcept { return head; }
    const_iterator end()    const noexcept { return tail; }
    const_iterator cbegin() const noexcept { return head; }
    const_iterator cend()   const noexcept { return tail; }

};



}



#endif
