/*
    ����ϲ���
    ע�����.h.cc���ļ�
    ��Ҫ��ĩβ����
    ��Ȼ�ᱨ��
    ����һ�����õĴ���淶
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
