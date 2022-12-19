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

#ifndef _COMPLEX_CC_
#define _COMPLEX_CC_
#include <cmath>

namespace sjtu {

struct complex {
  public:
    complex(double _r = 0,double _i = 0) : real(_r),imag(_i) {}
    ~complex() = default;

    double real,imag;
    /* The abs value of a complex object. */ 
    inline friend double abs(const complex &X) {
        return X.real * X.real + X.imag * X.imag;
    }
    inline friend complex operator +(const complex &X,const complex &Y) {
        return complex(X.real + Y.real , X.imag + Y.imag);
    }
    inline friend complex operator -(const complex &X,const complex &Y) {
        return complex(X.real - Y.real , X.imag - Y.imag);
    }
    inline friend complex operator *(const complex &X,const complex &Y) {
        return complex(X.real * Y.real - X.imag * Y.imag,
                       X.real * Y.imag + X.imag * Y.real);
    }
    inline friend complex operator /(const complex &X,const complex &Y) {
        double den = abs(Y);
        return complex((X.real * Y.real + X.imag * Y.imag) / den,
                       (X.real * Y.imag - X.imag * Y.real) / den);
    }

    /**
     * @brief  Change a complex object into its conjugate complex number.
     * 
     * @return The conjugate of the previous complex.
     */
    inline complex &conjugate(void) {
        imag = -imag;
        return *this;
    }

    /* Change a complex object into its conjugate complex number. */
    inline friend void conjugate(complex &X) {
        X.imag = -X.imag;
    }

    /*  Return the conjugate complex number. */
    inline friend complex operator ~(const complex &X) {
        return complex(X.real,-X.imag);
    }
    /* Return the conjugate complex number. */
    inline friend complex operator ~(complex && X) {
        return X.conjugate();
    }


    inline friend complex &operator +=(complex &X,const complex & Y) {
        X.real += Y.real;
        X.imag += Y.imag;
        return X;
    }
    inline friend complex &operator -=(complex &X,const complex & Y) {
        X.real -= Y.real;
        X.imag -= Y.imag;
        return X;
    }
    inline friend complex &operator *=(complex &X,const complex & Y) {
        return X = X * Y;
    }
    inline friend complex &operator /=(complex &X,const complex & Y) {
        return X = X / Y;
    }

    inline void set(double sita) {
        real = cos(sita);
        imag = sin(sita);
    }


};
constexpr double PI = 3.141592653589793238462643383;


}

#endif

#ifndef _INT2048_H_
#define _INT2048_H_


#ifndef NUMBER_TYPE
#define NUMBER_TYPE 0 // Default : NTT
#endif


#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>

namespace sjtu {


/**
 * @brief Fast built-in LOG2 function from
 * "骆可强《论程序底层优化的一些方法与技巧"
 * You can access that at 
 * https://github.com/lzyrapx/Competitive-Programming-Docs/.
 * 
 * 
 * @return log2(x) in 32-bit.
 */
inline unsigned int LOG2(unsigned x){
    unsigned ret;
    __asm__ __volatile__ ("bsrl %1, %%eax":"=a"(ret):"m"(x));
    return ret;
}



class Number_base {
  protected:
    Number_base() = default;
    // The minimum length for Newton Method division.
    static constexpr uint64_t DIV_minLen  = 8;
    static constexpr int32_t  judgeMap[2] = {1 , -1};
    static constexpr uint64_t MULT_minLen = 12;

    /* Get the reverse vector.(With small optimization) */
    static void getRev (array <size_t> &rev,size_t len) {
        static size_t last = -1;
        if(len == last) return;
        rev.resize(len);
        rev[0] = 0;
        for(size_t i = 1 ; i < len ; ++i) {
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) * len >> 1);
        }
    }
};

/**
 * @brief NTT related data.
 * You can't apply for an NTT_base object. 
 * 
 */
class NTT_base : protected Number_base{
  protected:
    NTT_base() = default;

    static constexpr uint64_t initLen     = 2  ;
    // static constexpr uint64_t MULT_minLen = 12 ;
    static constexpr uint64_t baseBit     = 5  ;
    static constexpr uint64_t base        = 1e5;
    static constexpr uint64_t unit[baseBit] = {
        1,10,100,1000,10000
    };
    

    // The maximum 01bits for NTT multiplication 
    static constexpr uint64_t NTT_maxBit = 27;
    // The maximum length for NTT multiplication
    static constexpr uint64_t NTT_maxLen = 1 << NTT_maxBit;
    // NTT mod numbers.
    static constexpr uint64_t mod[2] = {2281701377,3489660929};
    // Unit roots for NTT
    static uint64_t root[2][2][NTT_maxBit];
    // Inverses for NTT
    static uint64_t inverse[2][NTT_maxBit];


    // Primary_roots
    static constexpr uint64_t rootP[2][2] = {
        3,(mod[0] + 1) /3,3,(mod[1] + 1)/3
    };


    static void NTT_init();
    template <bool type>
    static void NTT (uint64_t *A,size_t len,bool opt = 0) {
        size_t cnt = 0;
        #define mod mod[type]
        for(size_t i = 1; i < len; i <<= 1) {
            uint64_t wn = root[type][opt][cnt++];
            for(size_t j = 0; j < len; j += (i << 1)) {
                uint64_t w = 1; // current w for NTT.
                for(size_t k = 0; k < i; ++k) {
                    uint64_t x = A[j | k];
                    uint64_t y = A[j | k | i] * w % mod;
                    A[j | k]     = (x + y)       % mod;
                    A[j | k | i] = (x - y + mod) % mod;
                    w = w * wn % mod;
                }
            }
        }
        #undef mod
    }
    template <bool type>
    static void INTT(uint64_t *A,size_t len) {
        NTT <type> (A,len,1);
    }
    template <bool type>
    static uint64_t quickPow(uint64_t base,uint64_t pow) {
        uint64_t ans = 1;
        while(pow) {
            if(pow & 1) ans = (ans * base) % mod[type];
            base = (base * base) % mod[type];
            pow >>= 1;
        }
        return ans;
    }

    static void reverse(uint64_t *A,size_t *rev,size_t len);
};
uint64_t NTT_base::root[2][2][NTT_base::NTT_maxBit];
uint64_t NTT_base::inverse[2][NTT_base::NTT_maxBit];

/**
 * @brief FFT related data.
 * You can't apply for an NTT_base object. 
 * 
 */
class FFT_base : protected Number_base {
  protected:
    FFT_base() = default;

    static constexpr uint64_t initLen     = 2  ;
    static constexpr uint64_t baseBit     = 3  ;
    static constexpr uint64_t base        = 1e3;
    static constexpr uint64_t unit[baseBit] = {
        1,10,100
    };

    static uint64_t Round(double num);
    // The maximum 01bits for NTT multiplication 
    static constexpr uint64_t FFT_maxBit = 20;
    // The maximum length for NTT multiplication
    static constexpr uint64_t FFT_maxLen = 1 << FFT_maxBit;

    // Unit roots for FFT
    static complex root[FFT_maxBit];

    static void FFT_init();

    static void FFT (complex *A,size_t len,bool opt);
    static void IFFT(complex *A,size_t len);
    static void reverse(complex *A,size_t *rev,size_t len);
};
complex FFT_base::root[FFT_maxBit];


/**
 * @brief Custom integer type.
 * 
 */
class int2048 : private array <uint64_t>,
#if NUMBER_TYPE != 1
    private NTT_base 
#else 
    private FFT_base
#endif
{
  private:
    static std::string buffer; // buffer inside
    // int2048(int2048 &&X,size_t len);
    int2048(const int2048 &X,size_t len);
    int2048(const array <uint64_t> &X,bool _sign);
    int2048(array<uint64_t> &&X,bool _sign);


    friend int2048 operator ~(const int2048 &X);

    friend int32_t Abs_Compare(const int2048 &X,const int2048 &Y);

    friend int2048 &SelfAdd(int2048 &X);
    friend int2048 &SelfSub(int2048 &X);
    friend int2048 &Add(int2048 &X,const int2048 &Y);
    friend int2048 &Sub(int2048 &X,const int2048 &Y);
    friend int2048 &Sub(const int2048 &Y,int2048 &X);
    friend int2048 &Mult_BF(int2048 &X,const int2048 &Y);
    friend int2048 &Mult_FT(int2048 &X,const int2048 &Y);
    friend int2048 DIV_BF(const int2048 &X,const int2048 &Y);
    friend int2048 DIV_NT(const int2048 &X,const int2048 &Y);
  public:
    /* Sign of the number.
       True if negative || False if positive.*/
    bool sign;


    int2048(int2048 &&tmp);
    int2048(const int2048 &tmp);
    int2048(intmax_t tmp);
    int2048(const std::string &str);
    ~int2048() = default;

    friend int2048 &operator ++(int2048 &X);
    friend int2048 &operator --(int2048 &X);
    

    friend int2048 operator +(const int2048 &X,const int2048 &Y);
    friend int2048 operator -(const int2048 &X,const int2048 &Y);
    friend int2048 operator *(const int2048 &X,const int2048 &Y);
    friend int2048 operator /(const int2048 &X,const int2048 &Y);
    friend int2048 operator %(const int2048 &X,const int2048 &Y);

    friend bool operator ==(const int2048 &X,const int2048 &Y);
    friend bool operator !=(const int2048 &X,const int2048 &Y);
    friend bool operator < (const int2048 &X,const int2048 &Y);
    friend bool operator <=(const int2048 &X,const int2048 &Y);
    friend bool operator > (const int2048 &X,const int2048 &Y);
    friend bool operator >=(const int2048 &X,const int2048 &Y);
    friend bool operator!(const int2048 &X);

    int2048 &reverse();
    friend int2048  operator -(int2048 &&X);
    friend int2048  operator -(const int2048 &X);
    friend int2048& operator +=(int2048 &X,const int2048 &Y);
    friend int2048& operator -=(int2048 &X,const int2048 &Y);
    friend int2048& operator *=(int2048 &X,const int2048 &Y);
    friend int2048& operator /=(int2048 &X,const int2048 &Y);
    friend int2048& operator %=(int2048 &X,const int2048 &Y);


    friend int2048 operator <<(const int2048 &X,const int64_t Y);
    friend int2048 operator >>(const int2048 &X,const int64_t Y);
    
    void read(const std::string &str);
    void print(std::ostream &os) const;
    
    
    friend std::istream &operator >>(std::istream &is,int2048 &dst);
    friend std::ostream &operator <<(std::ostream &os,const int2048 &src);
    int2048 &operator =(int2048 &&tmp);
    int2048 &operator =(const int2048 &tmp);

    explicit operator bool() const;
    explicit operator double() const;
    
    // ~int2048() {std::cout <<"Deallocate:"<<size()<<'\n';}
};


std::string int2048::buffer = ""; // Buffer for input and output.

const size_t SIZEOFINT2048 = sizeof(NTT_base);


}
#endif

#ifndef _FFT_CC_
#define _FFT_CC_

#if NUMBER_TYPE == 1



namespace sjtu {

/* Round a double number to uint64_t */
inline uint64_t FFT_base::Round(double num) {
    return num + 0.5;
}

/* Initialize function for FFT.Called once! */
inline void FFT_base::FFT_init() {
    for(size_t i = 0 ; i < FFT_maxBit ; ++i) {
        root[i].set(PI / double(1ULL << i));
    }
}

/* Perform "butterfly operation". */
inline void FFT_base::reverse(complex *A,size_t *rev,size_t len) {
    for(size_t i = 0 ; i < len ; ++i) {
        if(i < rev[i]) std::swap(A[i],A[rev[i]]);
    }
}


void FFT_base::FFT(complex *A,size_t len,bool opt = 0) {
    size_t cnt = 0;
    for(size_t i = 1 ; i < len ; i <<= 1) {
        complex wn = root[cnt++]; // unit root
        if(opt) conjugate(wn);
        for(size_t j = 0 ; j < len ; j += (i << 1)) {
            complex w(1.0,0.0);
            for(size_t k = 0 ; k < i ; ++k , w *= wn) {
                complex x = A[j | k] ;
                complex y = A[j | k | i] * w;
                A[j | k]     = x + y;
                A[j | k | i] = x - y;
            }
        }
    }

}

inline void FFT_base::IFFT(complex *A,size_t len) {
    return FFT(A,len,1);
}

/* Multiplication based on Fast Fourier Transformation. */
int2048 &Mult_FT(int2048 &X,const int2048 &Y) {
    X.sign ^= Y.sign;

    size_t maxLen = X.size() + Y.size();
    size_t len    = 1 << (LOG2(maxLen - 1) + 1);

    static array <complex> A;
    A.resize(len);

    {// Make out A and avoid too much logical judgement.
        size_t i = 0;
        if(X.size() > Y.size()) {
            while(i < Y.size()) { A[i].real = X[i],A[i].imag = Y[i]; ++i; }
            while(i < X.size()) { A[i].real = X[i],A[i].imag = 0;    ++i; }
        } else { // Y.size() >= X.size()
            while(i < X.size()) { A[i].real = X[i],A[i].imag = Y[i]; ++i; }
            while(i < Y.size()) { A[i].imag = Y[i],A[i].real = 0;    ++i; }
        }
        while(i < len) { A[i].real = A[i].imag = 0; ++i; }
    }

    static array <size_t> rev;
    int2048::getRev(rev,len);

    FFT_base::reverse(A.begin(),rev.begin(),len);
    FFT_base::FFT(A.begin(),len);

    for(size_t i = 0 ; i < len ; ++i) { A[i] *= A[i]; }

    FFT_base::reverse(A.begin(),rev.begin(),len);
    FFT_base::IFFT(A.begin(),len);

    X.resize(maxLen);
    uint64_t ret = 0;

    len <<= 1;
    for(size_t i = 0 ; i < maxLen ; ++i) {
        ret += FFT_base::Round(A[i].imag / double(len));
        X[i] = ret % int2048::base;
        ret /= int2048::base;
    }
    while(!X.back()) X.pop_back();
    return X;
}



}

#endif
#endif
#ifndef _NTT_CC_
#define _NTT_CC_


#if NUMBER_TYPE != 1


namespace sjtu {

/* Initialize NTT_base's object */
inline void NTT_base::NTT_init() {
    for(size_t i = 0 ; i != NTT_maxBit ; ++i) {
        root[0][0][i] = quickPow <0> (rootP[0][0],(mod[0] - 1) / (1 << (i + 1)));
        root[0][1][i] = quickPow <0> (rootP[0][1],(mod[0] - 1) / (1 << (i + 1)));
        root[1][0][i] = quickPow <1> (rootP[1][0],(mod[1] - 1) / (1 << (i + 1)));
        root[1][1][i] = quickPow <1> (rootP[1][1],(mod[1] - 1) / (1 << (i + 1)));
        inverse[0][i] = quickPow <0> (1ULL << i,mod[0] - 2);
        inverse[1][i] = quickPow <1> (1ULL << i,mod[1] - 2);
    } // test passed
}

/* Perform "butterfly operation". */
inline void NTT_base::reverse(uint64_t *A,size_t *rev,size_t len) {
    for(size_t i = 0 ; i < len ; ++i) {
        if(i < rev[i]) std::swap(A[i],A[rev[i]]);
    }
}

int2048 &Mult_FT(int2048 &X,const int2048 &Y) {
    X.sign ^= Y.sign;

    size_t maxLen = X.size() + Y.size();
    size_t len    = LOG2(maxLen - 1) + 1;

    const uint64_t inv[2] = {
        NTT_base::inverse[0][len],
        NTT_base::inverse[1][len]
    };

    constexpr uint64_t invMod = 1014089499ULL;

    len = 1ULL << len;

    array <uint64_t> A0(std::move(X),len);
    array <uint64_t> B0(Y,len);

    static array <size_t> rev;
    int2048::getRev(rev,len);

    NTT_base::reverse(A0.begin(),rev.begin(),len);
    NTT_base::reverse(B0.begin(),rev.begin(),len);

    array <uint64_t> A1 = A0;
    array <uint64_t> B1 = B0;

    int2048::NTT <0> (A0.begin(),len);
    int2048::NTT <1> (A1.begin(),len);
    int2048::NTT <0> (B0.begin(),len);
    int2048::NTT <1> (B1.begin(),len);

    for(size_t i = 0 ; i < len ; ++i) {
        A0[i] = (A0[i] * B0[i]) % int2048::mod[0]; 
        A1[i] = (A1[i] * B1[i]) % int2048::mod[1];
    }

    NTT_base::reverse(A0.begin(),rev.begin(),len);
    NTT_base::reverse(A1.begin(),rev.begin(),len);

    int2048::INTT <0> (A0.begin(),len);
    int2048::INTT <1> (A1.begin(),len);

    uint64_t ret = 0;
    for(size_t i = 0 ; i < maxLen ; ++i) {
        A0[i] = (A0[i] * inv[0]) % int2048::mod[0];
        A1[i] = (A1[i] * inv[1]) % int2048::mod[1];
        ret += (A0[i] == A1[i]) ?
                A0[i] : (A0[i] - A1[i] + int2048::mod[0] * 2) * invMod
                     % int2048::mod[0] * int2048::mod[1] + A1[i];
        A0[i] = ret % int2048::base;
        ret  /= int2048::base;
    }
    X.swap(A0);
    X.resize(maxLen);
    if(!X.back()) X.pop_back();
    return X;
}



}

#endif
#endif
#ifndef _INT2048_CC_
#define _INT2048_CC_



/* Other operator part */
namespace sjtu {

struct int2048_initializer : private NTT_base,FFT_base{
    int2048_initializer() {
#if NUMBER_TYPE != 1
        NTT_init();
#else 
        FFT_init();
#endif
    }
}_THIS_OBJECT_IS_CONSTRUCTED_TO_INITIALIZE_;


}





/* Logical operator part.*/
namespace sjtu {

/**
 * @brief Compare the absolute value of X and Y.
 * 
 * @return -1 if abs(X) < abs(Y) ||
 *          0 if abs(X) = abs(Y) ||
 *          1 if abs(X) > abs(Y)
 */
int32_t Abs_Compare(const int2048 &X,const int2048 &Y) {
    if(X.size() != Y.size()) {
        return int2048::judgeMap[X.size() < Y.size()];
    }

    // X.size() == Y.size()
    for(size_t i = X.size() -1 ; i != -1ULL ; --i) {
        if(X[i] != Y[i]) {
            return int2048::judgeMap[X[i] < Y[i]];
        }
    }

    // abs(X) == abs(Y)
    return 0; 
}

/* Compare X and Y*/
inline bool operator <(const int2048 &X,const int2048 &Y) {
    if(X.sign != Y.sign) return X.sign > Y.sign;
    else return Abs_Compare(Y,X) == int2048::judgeMap[X.sign]; 
}
/* Compare X and Y*/
inline bool operator >(const int2048 &X,const int2048 &Y) {
    if(X.sign != Y.sign) return X.sign < Y.sign;
    else return Abs_Compare(X,Y) == int2048::judgeMap[X.sign];
}
/* Compare X and Y*/
inline bool operator <=(const int2048 &X,const int2048 &Y) {
    if(X.sign != Y.sign) return X.sign > Y.sign;
    else return Abs_Compare(X,Y) != int2048::judgeMap[X.sign];
}
/* Compare X and Y*/
inline bool operator >=(const int2048 &X,const int2048 &Y) {
    if(X.sign != Y.sign) return X.sign < Y.sign;
    else return Abs_Compare(Y,X) != int2048::judgeMap[X.sign];
}
/* Compare X and Y*/
inline bool operator ==(const int2048 &X,const int2048 &Y) {
    if(X.sign != Y.sign || X.size() != Y.size()) return false;
    return !Abs_Compare(X,Y);
}

/* Compare X and Y*/
inline bool operator !=(const int2048 &X,const int2048 &Y) {
    if(X.sign != Y.sign || X.size() != Y.size()) return true;
    return Abs_Compare(X,Y);
}

/* Compare X and Y*/
inline bool operator !(const int2048 &X) {
    return !X.back();
}


}





/* Arithmetic operator part. */
namespace sjtu {



}





#endif

