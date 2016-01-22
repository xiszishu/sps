#include <stdio.h>
#include <string>
#include <iostream>
using namespace std;
int main()
{
    char c;
    string s;
    int i,j;
    freopen("a.in","r",stdin);
    freopen("a.out","w",stdout);
    while (!cin.eof())
    {
        getline(cin,s);
        //cout<<s<<endl;
        for (i=0;i<=19;i++)
        if ((s[i]<='9')&&(s[i]>='0'))
        {
            putchar(s[i]);
        }
        putchar(' ');
    }
    return 0;
}
