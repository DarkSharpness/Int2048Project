#ifndef _NTT_CC_
#define _NTT_CC_


#include "int2048.h"
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
