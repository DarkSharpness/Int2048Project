#ifndef _INT2048_H_
#define _INT2048_H_


#ifndef NUMBER_TYPE
#define NUMBER_TYPE 0 // Default : NTT
#endif


#include "complex.cc"
#include "vector.cc"
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

