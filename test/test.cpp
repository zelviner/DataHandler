/// @file test.cpp
/// @author ZEL (zel1362848545@gmail.com)
/// @brief
/// @version 0.1
/// @date 2023-12-04
///
/// @copyright Copyright (c) 2023 ZEL
///

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

void swap(int *a, int *b) {
    int temp = *a;
    *a       = *b;
    *b       = temp;
}

int main() {

    int a = 10, b = 20;
    std::cout << "a = " << a << ", b = " << b << std::endl;

    swap(&a, &b);
    std::cout << "a = " << a << ", b = " << b << std::endl;

    std::ifstream in("test.txt");

    std::string line;

    std::vector<std::string> lines;

    std::map<std::string, int> map;

    return 0;
}
