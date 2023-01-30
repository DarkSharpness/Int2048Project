// HEADER-ONLY library for complex number.
// It's a simple template for complex number.
// Don't use it unless neccesary.
#ifndef _COMPLEX_TCC_
#define _COMPLEX_TCC_

#include <cmath>

namespace dark {


/**
 * @brief Basic complex number , implemented by 
 * separately storing the real and imaginary part.
 * 
 * @tparam value_t Value type of internal data.(Default double)
 */
template <class value_t = double>
struct complex {
    value_t real; // Real part.
    value_t imag; // Imaginery part.

    ~complex() noexcept = default;

    /* Initialize with real part and imaginery part.*/
    complex(value_t _r = 0.0,value_t _i = 0.0) noexcept : real(_r),imag(_i) {}
    
    /* Copy construction noexcept. */
    complex(const complex &rhs) noexcept = default;

    /* Initialize with another complex number. */
    template <class value_t_2>
    complex(const complex <value_t_2> &rhs) noexcept : real(rhs.real),imag(rhs.imag) {}

    /* Assign operation. */
    complex &operator = (const complex &rhs) noexcept = default;

    /* Assign operation. */
    template <class value_t_2>
    complex &operator = (const complex <value_t_2> &rhs) noexcept {
        real = rhs.real;
        imag = rhs.imag;
        return *this;
    }


    /* Return the square of abs.*/
    value_t abs2() const noexcept { return real * real + imag * imag; }    
    /* Return the square of abs.*/
    friend value_t abs2(const complex &rhs) noexcept { return rhs.abs2(); }

    /* Return the abs value. */
    double abs() const { return sqrt(abs2()); }
    /* Return the abs value. */
    friend double abs(const complex &rhs)  { return rhs.abs(); }

    /* Conjugate the complex number. 
       It will reverse the imaginery part.*/
    complex &conjugate() noexcept { imag = -imag;return *this; } 
    /* Return conjugate of the value. */
    friend complex conjugate(complex &&rhs) noexcept {
        return rhs.conjugate();
    }
    /* Return conjugate of the value. */
    friend complex conjugate(const complex &rhs) noexcept {
        return complex(rhs).conjugate();
    }

    /* Set the argument of a complex number in given length(default 1). */
    void set(double arg,value_t len = 1.0) noexcept {
        real = len * cos(arg);
        imag = len * sin(arg);
    }
    /* Return the argument of a complex number.
       The argument ranges from -pi/2 to pi/2.  */
    double argument() noexcept {
        return atan(imag / real);
    }

    /* Arithmetic addition assign. */
    template <class value_t_2>
    complex &operator +=(const complex <value_t_2> &rhs) noexcept {
        real += rhs.real;
        imag += rhs.imag;
        return *this;
    }
    /* Arithmetic addition. */
    friend complex operator +(const complex &lhs,const complex &rhs) noexcept {
        return complex(lhs.real + rhs.real,lhs.imag + rhs.imag);
    }

    /* Arithmetic subtraction assign. */
    template <class value_t_2>
    complex &operator -=(const complex <value_t_2> &rhs) noexcept {
        real -= rhs.real;
        imag -= rhs.imag;
        return *this;
    }
    /* Arithmetic subtraction. */
    friend complex operator -(complex lhs,const complex &rhs) noexcept {
        return complex(lhs.real - rhs.real,lhs.imag - rhs.imag);
    }

    /* Arithmetic multiplication assign.
       Note that it is the same as lhs = lhs * rhs. */
    complex &operator *=(const complex &rhs) noexcept {
        return *this = *this * rhs;
    }
    /* Arithmetic multiplication. */
    friend complex operator *(const complex &lhs,const complex &rhs) noexcept {
        return complex(lhs.real * rhs.real - lhs.imag * rhs.imag,
                       lhs.real * rhs.imag + lhs.imag * rhs.real);
    }

    /* Arithmetic division assign.
       Note that it is the same as lhs = lhs * rhs. */
    complex &operator /=(const complex &rhs) noexcept{
        return *this = *this / rhs;
    }
    /* Arithmetic division. */    
    friend complex operator /(const complex &lhs,const complex &rhs) noexcept {
        value_t tmp = rhs.abs2();
        return complex((lhs.real * rhs.real + lhs.imag * rhs.imag) / tmp,
                       (lhs.real * rhs.imag - lhs.imag * rhs.real) / tmp);
    }

};



}

#endif
