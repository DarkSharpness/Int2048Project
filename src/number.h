#ifndef _DARK_NUMBER_H_
#define _DARK_NUMBER_H_

#define NTT_MODE 1
#define FFT_MODE 0

#ifndef NUMBER_MODE
#define NUMBER_MODE FFT_MODE // Default mode is FFT
#endif


#include "array.h"
#include <array>

namespace dark::number {

/* Get the reverse vector.(With optimization) */
void getRev (Array <size_t> &rev,size_t len) { 
    static size_t last = -1;
    if(len == last) return;
    rev.resize(len);
    rev[0] = 0;
    for(size_t i = 1 ; i != len ; ++i) { // len != 0
        rev[i] = (rev[i >> 1] >> 1) | ((i & 1) * len >> 1);
    }
}

/* Simple constexpr pow function. */
template <class T>
constexpr T Cpow(const T &base,size_t pow) {
    T ans = 1;
    while(pow--) ans *= base;
    return ans;
}

/* Round to the nearest integer. */
inline constexpr size_t round(double num) { return size_t(num + 0.5); }



// template <size_t length>
// constexpr std::array <size_t,length> init() {
//     std::array <size_t,length> ans;
//     ans[0] = 1;
//     for(size_t i = 1 ; i != length ; ++i) // len != 0;
//         ans[i] = ans[i - 1] * 10;
//     return ans;
// }



}


#endif
