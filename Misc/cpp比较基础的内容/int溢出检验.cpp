//
// Created by fzhinvg on 2025/4/20.
//
#include <iostream>
#include <stdckdint.h> // c23

template<typename T>
bool checked_add(T a, T b, T &result)
{
    return __builtin_add_overflow(a, b, &result);
}

int main()
{
//    int a = 2147483647;
    int a = 0x7fffffff;
    int b = 1;
    int result;

    if (ckd_add(&result, a, b))
    {
        std::cerr << "Overflow detected!\n";
    } else
    {
        std::cout << "Result: " << result << "\n";
    }
    return 0;
}