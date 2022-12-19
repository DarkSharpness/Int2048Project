#ifndef _VECTOR_CC_
#define _VECTOR_CC_
#include <bits/allocator.h>
#include <stdint.h>
#include <string.h>
#include <initializer_list>

namespace sjtu {

/**
 * @brief An array that don't construct/deconstruct element
 * and don't automatically manage space.
 * It's easy to use with fairly high efficiency.
 * 
 */
template <typename value_t>
class array : private std::allocator <value_t> {
  private:
    value_t *head; /*Head pointer*/
    value_t *tail; /*Tail pointer*/
    value_t *term; /*Terminal pointer*/

  public:
    array() {
        tail = head = this->allocate(1);
        term = head + 1;
    }
    /* Copy data from Y. */
    array(const array &Y) {
        head = tail = term = nullptr;
        copy(Y);
    }
    /* Initialize from Y and fill back with 0 to siz*/
    array(const array &Y,size_t siz) {
        head = tail = term = nullptr;
        reserve(siz);
        tail = head + Y.size();
        memcpy(head,Y.head, Y.size() * sizeof(value_t));
        memset(tail,0,(siz - size()) * sizeof(value_t));
        tail = head + siz;
    }

    /* Initialize from Y and fill front with 0 to siz*/
    array(int64_t siz,const array &Y) {
        head = tail = term = nullptr;
        if(siz > 0) {
            reserve(siz + Y.size());
            memset(head,0,siz * sizeof(value_t));
            memcpy(head + siz,Y.head,Y.size() * sizeof(value_t));
            tail = term;
        } else if(siz < 0) {
            if(siz + Y.size() <= 0) return; // empty
            reserve(siz + Y.size());
            memcpy(head,Y.head - siz,(siz + Y.size()) * sizeof(value_t));
            tail = term;
        } else { /* siz = 0*/
            copy(Y);
        }
    }

    /* Move data from Y. */
    array(array &&Y) {
        head = tail = term = nullptr;
        swap(Y);
    }

    /* Initialize from Y and fill back with 0 to siz*/
    array(array &&Y,size_t siz) {
        head = tail = term = nullptr;
        std::swap(head,Y.head);
        std::swap(tail,Y.tail);
        std::swap(term,Y.term);
        expand_back(siz - size());
    }

    array(std::initializer_list <value_t> list) {
        if(!list.size()) {
            tail = head = this->allocate(1);
            term = head + 1;
            return;
        } else {
            head = this->allocate(list.size());
            tail = term = head + list.size();
        }
        memcpy(head,list.begin(),list.size() * sizeof(value_t));  
    }
    
    ~array() {
        this->deallocate(head,capacity());
    }

    inline bool empty() const{
        return head == tail;
    }

    /* Return the count of elements in the array */
    inline size_t size() const{
        return tail - head;
    }
    
    /* Return the maximum elements the array can hold
       before next allocation.*/
    inline size_t capacity() const{
        return term - head;
    }

    /* Return the remaining space of the array
       before next allocation.*/
    inline size_t remainder() const{
        return term - tail;
    }

    /**
     * @brief Reserve space for siz elements.
     * If currrent capacity() > cap, nothing is done.
     * 
     * @param cap The target capacity() to be reached.
     */
    inline void reserve(size_t cap) {
        if(cap > capacity()) {
            value_t *temp = this->allocate(cap);
            memcpy(temp,head,size() * sizeof(value_t));
            this->deallocate(head,capacity());
            tail += temp - head;
            head  = temp;
            term  = temp + cap;
        }
    }

    /**
     * @brief Change the size() to siz.
     * Note that the elements within are not initialized.
     * To use it,you should initialize it yourself.
     * 
     * @param siz The target size() to be reached.
     */
    inline void resize(size_t siz) {
        reserve(siz);
        tail = head + siz;
    }

    /**
     * @brief Push an element to the back of the array.
     * It behaves like a stack operation.
     * 
     * @param val The value to be pushed_back.
     */
    inline void push_back(const value_t &val) {
        if(tail == term) reserve(capacity() * 2);
        *(tail++) = val;
    }

    /**
     * @brief Push an element to the back of the array.
     * It behaves like a stack operation.
     * You should allocate the space beforehand.
     * 
     * @param val The value to be pushed_back.
     */
    inline void emplace_back(const value_t &val) {
        *(tail++) = std::move(val);
    }

    /* Shrink the size() by 1*/
    inline void pop_back() {
        --tail;
    }

    /**
     * @brief Expand the front by count and fill with 0.
     * It's a costly O(n) operation.
     * 
     * @param count The number of space in the front.
     */
    inline void expand_front(size_t count = 1) {
        if(count <= remainder()) {
            memmove(head + count,head,size() * sizeof(value_t));
        } else {
            value_t *temp = this->allocate(size() + count);
            memcpy(temp + count,head,size() * sizeof(value_t));
            this->deallocate(head,capacity());
            term = tail += temp - head + count;
            head = temp;
        }
        memset(head,0,count * sizeof(value_t));
    }

    /**
     * @brief Expand the back by count and fill with 0.
     * It's an average costly O(count) operation.
     * 
     * @param count The number of space in the front.
     */
    inline void expand_back(size_t count = 1) {
        if(count > remainder()) {reserve(count + size());}
        memset(tail,0,count * sizeof(value_t));
        tail += count;
    }


    /* Copy the data from Y. No deconstruction is done.*/
    inline array &operator =(const array &Y) {
        copy(Y);
        return *this;
    }

    /* Swap the data with Y. No deconstruction is done.*/
    inline array &operator =(array &&Y) {
        swap(Y);
        return *this;
    }

    /* Copy the data from Y. No deconstruction is done.*/
    inline void copy(const array &Y) {
        if(this == &Y) return;
        if(Y.size() > capacity()) {
            this->deallocate(head,capacity());
            term = Y.size() + (head = this->allocate(Y.size()));
        }
        tail = Y.size() + head;
        memcpy(head,Y.head,Y.size() * sizeof(value_t));
    }

    /* Swap the data with Y. No deconstruction is done.*/
    inline void swap(array &Y) {
        if(this == &Y) return;
        std::swap(head,Y.head);
        std::swap(tail,Y.tail);
        std::swap(term,Y.term);
    }
    /* Clear the elements from the array. No deconstruction is done. */
    inline void clear() {
        tail = head;
    }

    /* Return reference to the idx's element in the vector. */
    inline value_t &operator [](size_t idx) {
        return head[idx];
    }

    /* Return const reference to the idx's element in the vector. */
    inline const value_t &operator [](size_t idx) const{
        return head[idx];
    }

    inline value_t *begin() {
        return head;
    }
    inline const value_t *begin() const{
        return head;
    }
    inline value_t *end() {
        return tail;
    }
    inline const value_t *end() const{
        return tail;
    }

    inline value_t &front() {
        return *head;
    }
    inline const value_t &front() const{
        return *head;
    }
    inline value_t &back() {
        return *(tail - 1);
    }
    inline const value_t &back() const{
        return *(tail - 1);
    }

};

// array <int> a;

}

#endif

