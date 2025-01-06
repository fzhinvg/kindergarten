//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>
#include <utility>

void process(int &x)
{
	std::cout << "Lvalue reference: " << x << std::endl;
}

void process(int &&x)
{
	std::cout << "Rvalue reference: " << x << std::endl;
}

template<typename T>
void forwardToProcess(T &&arg)// 这里不叫右值引用，叫万能引用或者转发引用，主要跟函数模板搭配使用
{
	process(std::forward<T>(arg));
	//std::forward：用于保持参数的左值或右值特性。它在模板函数中使用，以确保在转发参数时，参数的左值或右值属性不变，从而实现完美转发。
}

int main()
{
	int a = 10;
	int *pInt = new int{1};
	forwardToProcess(a);         // 传递左值
	forwardToProcess(*pInt);
	forwardToProcess(20);        // 传递右值
	return 0;
}