#ifndef _DARK_FFT_H_
#define _DARK_FFT_H_

#include "number.h"
#include "complex.tcc"

namespace dark::number {

constexpr double PI = 3.141592653589793238462643383; // Standard pi.

template <class T,size_t N>
std::array <complex<T>,N> make_root() {
    std::array <complex<T>,N> root;
    for(size_t i = 0 ; i < N ; ++i) {
        root[i].set(PI / double(1ULL << i));
    }
    return root;
}


/**
 * @brief Base data for big integer type int2048.
 * 
 */
class int_base {
  protected:
    int_base() = default;
    ~int_base() = default;

    static constexpr size_t baseLen = 3;                       // Base length in DEC.
    static constexpr size_t base = Cpow <size_t> (10,baseLen); // Base size.
    static constexpr size_t unit[baseLen] = {1,10,100};        // Unit below base.

    static constexpr size_t mulMin  = 10; // Minimum length for fast multiplication.
    static constexpr size_t divMin  = 10; // Minimum length for fast division.
    static constexpr size_t initLen = 10; // Initial length of a number.

  public:
    static constexpr size_t FFTLen = 20;                   // FFT max 01 bit length.
    static constexpr size_t FFTMAX = 1ULL << FFTLen;       // FFT max array  length.
    static const std::array <complex<double>,FFTLen> root; // root of FFT operation.


};
constexpr size_t int_base::baseLen;
constexpr size_t int_base::base;
constexpr size_t int_base::unit[baseLen];
constexpr size_t int_base::mulMin;
constexpr size_t int_base::divMin;
constexpr size_t int_base::initLen;
constexpr size_t int_base::FFTLen;
constexpr size_t int_base::FFTMAX;
const std::array <complex<double>,int_base::FFTLen> int_base::root = 
    make_root <double,int_base::FFTLen> ();

}

namespace dark {

    
class int2048 : private number::Array <uint64_t>,number::int_base {
  private:
    static std::string buffer;  // Inout buffer.

    int2048(const int2048 &X,size_t len);
    int2048(const Array <uint64_t> &X,bool _sign);
    int2048(Array <uint64_t> &&X,bool _sign);

    friend int2048 operator ~(const int2048 &X);
    friend int Abs_Compare(const int2048 &X,const int2048 &Y);

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

};

constexpr size_t INTSIZE = sizeof(int2048);



}





#endif
