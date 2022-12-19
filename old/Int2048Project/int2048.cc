#ifndef _INT2048_CC_
#define _INT2048_CC_


#include "int2048.h"
#include "FFT.cc"
#include "NTT.cc"

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

/**
 * @brief Initialize from a string.
 * 
 * @param str The string to initialize from.
 */
void int2048::read(const std::string &str) {
    if(str.front() == '-') sign = true;
    else                   sign = false;
    clear(); // clear the previous data.
    uint64_t j = sign;
    while(str[j] == '0') ++j; // clear prefix-0
    if(!str[j]) { // 0 case.
        sign = false;
        emplace_back(0);
        return;
    }
    reserve(1 + (str.size() - j) / baseBit);
    uint64_t i    = str.size();
    uint64_t cnt  = 0;
    uint64_t ret  = 0;
    while(i-- != j) { // Loop in [sign,str.size())
        ret += unit[cnt] * (str[i] ^ '0');
        if(++cnt == baseBit) {
            emplace_back(ret);
            ret = cnt = 0;
        }
    }
    if(cnt) emplace_back(ret);
}

/* Print out inside number(Default by std::cout)*/
void int2048::print(std::ostream &os = std::cout) const{
    if(sign) os << '-';
    os << back();
    buffer.resize((size() - 1) * baseBit);

    for(uint32_t i = 0 ; i < size() - 1 ; ++i) {
        uint64_t ret = (*this)[size() - 2 - i];
        for(uint32_t j = baseBit - 1 ; j != -1u ; --j) {
            buffer[i * baseBit + j] = (ret % 10) ^ '0';
            ret /= 10;
        }
    }

    os << buffer;
}

/* Swap data with another int2048. */
int2048 &int2048::operator =(int2048 &&tmp) {
    swap(tmp);
    sign = tmp.sign;
    return *this;
}


/* Copy data from another int2048. */
int2048 &int2048::operator =(const int2048 &tmp) {
    copy(tmp);
    sign = tmp.sign;
    return *this;
}


/* Instream input for an int2048 object. */
std::istream &operator >>(std::istream &is,int2048 &dst) {
    is >> int2048::buffer;
    dst.resize(0);
    dst.read(int2048::buffer);
    return is;
}


/* Outstream output for an int2048 object. */
std::ostream &operator <<(std::ostream &os,const int2048 &src) {
    src.print(os);
    return os;
}


/* Initialize from a long long object.(Default = 0)*/
int2048::int2048(intmax_t tmp = 0) {
    if(!tmp) { // tmp = 0
        sign = false;
        emplace_back(0);
        return;
    } else {
        reserve(initLen);
        if(tmp < 0) {
            sign = true;
            tmp  = -tmp;
        } else { // tmp > 0
            sign = false;
        }
        while(tmp) {
            push_back(tmp % base);
            tmp /= base;
        }
    }

}

/* Move construction. */
int2048::int2048(int2048 &&tmp) {
    swap(tmp);
    sign = tmp.sign;
}

/* Copy construction. */
int2048::int2048(const int2048 &tmp) {
    copy(tmp);
    sign = tmp.sign;
}

/* Initialize from std::string. */
int2048::int2048(const std::string &str) {
    read(str);
}

/* */
int2048::int2048(const int2048 &X,size_t len) {
    sign = X.sign;
    reserve(len);
    copy(X);
}

int2048::int2048(const array<uint64_t> &X,bool _sign) {
    sign = _sign;
    copy(X);
}

int2048::int2048(array<uint64_t> &&X,bool _sign) {
    sign = _sign;
    swap(X);
}

inline int2048::operator bool() const{
    return back();
}



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

int2048 operator <<(const int2048 &X,const int64_t Y) {
    int2048 ans = int2048(array <uint64_t> (Y,X),X.sign);
    if(ans.empty()) return 0;
    else return ans;
}


int2048 operator >>(const int2048 &X,const int64_t Y) {
    int2048 ans = int2048(array <uint64_t> (-Y,X),X.sign);
    if(ans.empty()) return 0;
    else return ans;
}

/* Add X's abs value by 1. */
int2048 &SelfAdd(int2048 &X) {
    for(size_t i = 0 ; i != X.size() ; ++i) {
        if(++X[i] < int2048::base) return X;
        X[i] = 0;
    }
    X.push_back(1);
    return X;
}

/* Subtract X's abs value by 1.
   Note that X != 0 */
int2048 &SelfSub(int2048 &X) {
    for(size_t i = 0 ; i != X.size() ; ++i) {
        if(--X[i] < int2048::base) break;
        X[i] = int2048::base;
    }
    while(!X.back()) X.pop_back();
    return X;
}

/* X = X + 1 */
int2048 &operator ++(int2048 &X) {
    if(X.sign && X.size() == 1 && X.back() == 1) {
        X[0] = X.sign = 0;
        return X;
    } return X.sign ? SelfSub(X) : SelfAdd(X);
}
/* X= X - 1*/
int2048 &operator --(int2048 &X) {
    if(X.back()) {
        X[0] = X.sign = 1;
        return X;
    } return X.sign ? SelfAdd(X) : SelfSub(X);
}


/* Reverse a number's sign.It will return itself. 
   Please use it instead of X *= (-1) or X = -X  */
inline int2048 &int2048::reverse() {
    sign ^= 1;
    return *this;
}

/* X * (-1) */
inline int2048 operator -(int2048 &&X) {
    return X.reverse();
}

/* X * (-1) */
inline int2048 operator -(const int2048 &X) {
    int2048 ans(X);
    return ans.reverse();
}


/**
 * @brief Add abs(X) by abs(Y) from X's front bit.
 * Make Sure X.size() >= Y.size()
 * 
 * @return X = sgn(X) * (abs(X) + abs(Y))
 */
int2048 &Add(int2048 &X,const int2048 &Y) {
    bool ret  = 0;
    size_t i  = 0;
    for(/* i = 0 */; i != Y.size() ; ++i) {
        X[i] += Y[i] + ret;
        ret   = X[i] >= int2048::base;
        if(ret) X[i] -= int2048::base;
    }
    if(ret) {
        while(i != X.size()) {
            if(X[i] != int2048::base - 1) {
                ++X[i];
                return X;
            } else {X[i++] = 0;}
        }
        X.push_back(1);
    }
    return X;
}

/**
 * @brief Subtract abs(Y) from abs(X) from X's front bit.
 * Make Sure abs(X) > abs(Y)
 * 
 * @return X = sgn(x) * (abs(X) - abs(Y))
 */
int2048 &Sub(int2048 &X,const int2048 &Y) {
    bool ret  = 0;
    size_t i  = 0;
    for(/* i = 0 */; i != Y.size() ; ++i) {
        X[i] -= Y[i] + ret;
        ret   = X[i] >> 63;
        if(ret) X[i] += int2048::base;   
    }
    if(ret) { 
        while(i != X.size()) {
            if(X[i]) {
                --X[i];
                break;
            } else {X[i++] = int2048::base - 1;}
        }
    }
    while(!X.back()) X.pop_back();
    return X;
}

/**
 * @brief Subtract abs(Y) from abs(X) from X's front bit.
 * Make Sure abs(X) < abs(Y)
 * 
 * @return X = sgn(x) * (abs(Y) - abs(X))
 */
int2048 &Sub(const int2048 &Y,int2048 &X) {
    X.expand_back(Y.size() - X.size());
    bool ret  = 0;
    size_t i  = 0;
    for(/* i = 0 */; i != X.size() ; ++i) {
        X[i]  = Y[i] - X[i] - ret;
        ret   = X[i] >> 63;
        if(ret) X[i] += int2048::base;   
    }
    if(ret) { 
        while(i != X.size()) {
            if(Y[i]) {
                X[i] = Y[i] - 1;
                break;
            } else {X[i++] = int2048::base - 1;}
        }
    }
    while(!X.back()) X.pop_back();
    return X;
}

/* X = X + Y */
int2048& operator +=(int2048 &X,const int2048 &Y) {
    if(X.sign == Y.sign) {
        if(X.size() < Y.size()) {
            X.expand_back(Y.size() - X.size() + 1);
            X.pop_back();
        }
        return Add(X,Y);
    } else {
        int32_t cmp = Abs_Compare(X,Y);
        if(!cmp) return X = 0;
        else if(cmp == -1) {
            return Sub(Y,X).reverse();
        } else { /*cmp == 1*/ 
            return Sub(X,Y);
        }
    }
}

/* X = X - Y */
int2048& operator -=(int2048 &X,const int2048 &Y) {
    if(X.sign != Y.sign) {
        if(X.size() < Y.size()) {
            X.expand_back(Y.size() - X.size() + 1);
            X.pop_back();
        }
        return Add(X,Y);
    } else {
        int32_t cmp = Abs_Compare(X,Y);
        if(!cmp) return X = 0;
        else if(cmp == -1) {
            return Sub(Y,X).reverse();
        } else { /*cmp == 1*/ 
            return Sub(X,Y);
        }
    }

}

/* X + Y */
int2048 operator +(const int2048 &X,const int2048 &Y) {
    if(X.size() < Y.size()) {
        int2048 ans(Y);
        return ans += X;
    } else {
        int2048 ans(X);
        return ans += Y;
    }
}

/* X - Y*/
int2048 operator -(const int2048 &X,const int2048 &Y) {
    if(X.size() < Y.size()) {
        int2048 ans(Y);
        ans.reverse();
        return ans += X;
    } else {
        int2048 ans(X);
        return ans -= Y;
    }
}

int2048 &operator *=(int2048 &X,const int2048 &Y) {
    if(!X) return X;
    if(!Y) return X = 0;
    // TODO : Brute Force Mult
    return Mult_FT(X,Y);
}

int2048 operator *(const int2048 &X,const int2048 &Y) {
    // TODO : Brute Force Mult
    if(!X || !Y) return 0;
    int2048 ans = 
#if NUMBER_TYPE != 1 // NTT needs at most 2 ^ n size()
    int2048(X,1 << (LOG2(X.size() + Y.size() - 1) + 1));
#else                // FFT only needs exact size()
    int2048(X,X.size() + Y.size());
#endif
    return Mult_FT(ans,Y);
}

int2048 operator /(const int2048 &X,const int2048 &Y) {
    int32_t cmp = Abs_Compare(X,Y);
    if(cmp == -1) return 0;
    if(cmp ==  0) return X.sign ^ Y.sign ? -1 : 1;

    uint64_t dif = X.size() - Y.size() * 2;
    if(int64_t(dif) < 0) dif = 0;

    // Y.size() + dif is the new length of Y.
    int2048 ans = ((X << dif) * ~(Y << dif)) >> (2 * (dif + Y.size()));
    ans.sign = false;

    // Small adjustments
    int2048 tmp = (ans + 1) * Y;
    while(Abs_Compare(tmp,X) != 1) {
        SelfAdd(ans);
        tmp += Y;
    }
    tmp = ans * Y;
    while(Abs_Compare(tmp,X) == 1) {
        SelfSub(ans);
        tmp -= Y;
    }
    ans.sign = X.sign ^ Y.sign;
    return ans;
}

int2048 &operator /=(int2048 &X,const int2048 &Y) {
    return X = X / Y;
}

/* The reverser of Y in twice the size() of Y.
   In other words , 1 / Y =  (~Y) / base ^ (2 * Y.size()) */
int2048 operator ~(const int2048 &X) {
    #define base int2048::base
    if(X.size() == 1) {
        int2048 ans; ans.pop_back();
        uint64_t i = base * base / X[0];
        while(i) {
            ans.push_back(i % base);
            i /= base;
        }
        return ans;
    } else if(X.size() == 2) {
        int2048 ans; ans.pop_back();
#if NUMBER_TYPE != 1
        constexpr uint64_t N = base * base * base; 
        uint64_t div = X[0] + X[1] * base;
        // i = base ^ 4 / div
        uint64_t i = (N / div) * base + ((N % div) * base) /div;
#else
        constexpr uint64_t N = base * base * base * base;
        uint64_t i = N / (X[0] + X[1] * base);
#endif
        while(i) { 
            ans.push_back(i % base);
            i /= base;
        }
        return ans;
    }
    #undef base

    size_t hf = 1 + (X.size() >> 1);     // half of X.size()
    int2048 Y = ~(X >> (X.size() - hf)); // First half bits reverse

    // Newton's method Y1 = Y0 * (2 - X * Y0).
    // When multiply Y0 ,it should be Y0 >> (hf << 1) 
    return 2 * (Y << (X.size() - hf)) - (X * Y * Y >> (hf << 1)); 
}


}





#endif

