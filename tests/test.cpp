#include <iostream>

using namespace std;

typedef int Calculate(int, int);

int add(int a, int b) { return a + b; }

int sub(int a, int b) { return a - b; }

int mul(int a, int b) { return a * b; }

int test(int a, int b, Calculate cal) { return cal(a, b); }

int main() {

    int num = test(100, 200, mul);

    cout << num << endl;

    printf("张恩乐！\n");

    return 0;
}