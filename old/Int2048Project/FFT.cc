#ifndef _FFT_CC_
#define _FFT_CC_
#include "int2048.h"

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
