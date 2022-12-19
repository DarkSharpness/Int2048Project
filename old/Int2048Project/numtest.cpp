#include "int2048.cc"

inline long long ran() {
    return ((long long)(rand()) << 32) + rand();
}

inline long long r() {
    return rand() & 1 ? rand() : -rand();
}

int main() {
    sjtu::int2048 x,y;
    srand(time(NULL));
    // std::cin >> x >> y;
    // std::cout << x / y;

    int n;
    long long a,b;
    std::cin >> n;
    while(n--) {
        x = a = r();
        y = b = r();
        if(a / b != x / y) {
            std::cout << a << ' ' << b;
            return 0;
        }
    }
    return 0;
}