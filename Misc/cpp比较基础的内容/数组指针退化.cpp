//
// Created by fzhinvg on 2025/7/1.
//
#include <iostream>
#include <typeinfo>
#include "kit.h"

using array_t = bool;

void function(const array_t *arr) {
    std::cout << sizeof(arr) << " " << typeid(arr).name() << std::endl;
    // 数组在函数参数中退化为指针
}

int main() {
data_init:
    int arr[10];
    for (int i = 0; i < 10; ++i) {
        arr[i] = i << 1;
    }

c_style:
    // access
    std::cout << *(arr + 9) << std::endl; // 等价于arr[9]
    std::cout << arr[9] << std::endl;
    std::cout << std::endl;

    // 数组类型的退化
    const array_t *array_t_ptr{};
    const array_t array_t_arr[128]{};

    function(array_t_ptr); // 传递指针
    std::cout << sizeof(array_t_ptr) << std::endl;
    kit::printSignature(array_t_ptr);
    std::cout << kit::getTypeName(array_t_ptr) << std::endl;
    std::cout << std::endl;
    function(array_t_arr); // 传递数组 - 这里数组退化为指向首元素的指针
    std::cout << sizeof(array_t_arr) << std::endl;
    kit::printSignature(array_t_arr);
    std::cout << kit::getTypeName(array_t_arr) << std::endl;


    return 0;
}
