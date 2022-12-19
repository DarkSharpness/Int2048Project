/*
    代码合并器
    注意你的.h.cc等文件
    需要有末尾空行
    不然会报错
    这是一个良好的代码规范
*/

#include<bits/stdc++.h>
using namespace std;
//#define int lont long

inline void write() {
    string str;
    cin.clear();
    while(getline(cin,str)) {
        if(str.substr(0,8) == "#include" && 
           (str[8] == '\"' || str[9] == '\"' || str[10] == '\"')) {
            continue;
        }
        cout << str << '\n';
    }
}

signed main() {
    freopen("final_submit.cpp","w",stdout);

    freopen("vector.cc","r",stdin);
    write();

    freopen("complex.cc","r",stdin);
    write();

    freopen("int2048.h","r",stdin);
    write();

    freopen("FFT.cc","r",stdin);
    write();

    freopen("NTT.cc","r",stdin);
    write();

    freopen("int2048.cc","r",stdin);
    write();
    return 0;
}
