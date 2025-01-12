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
// 新增:感觉这两个标准库函数将万能引用和引用折叠使用得淋漓尽致
#if 0
// part0:
// 模板中T&&万能引用接收到左右值的推导:
// 当传递给万能引用 T&& 的是左值时,模板参数 T 会被推导为左值引用类型
// 当传递给万能引用 T&& 的是右值时,模板参数 T 会被推导为基础类型 (没有引用属性)

// 如果传递左值 int&，T 被推导为 int&
// 因此 T&& 变为 int& &&
// 根据引用折叠规则,int& && 会折叠为 int&

// 如果传递右值 int&& 或者 int
// T 被推导为基础类型 int,因此 T&& 变为 int&&

//-------------------------------------------------------------------
// part1:
// forward和move是怎么处理左右值的:
template<typename T>
T&& forward(typename std::remove_reference<T>::type& t) noexcept {
	return static_cast<T&&>(t);
}

int x = 10;
forward<int&>(x); // L
// 1.模板参数推导: T 被推导为 int&
// 2.剥离引用属性: std::remove_reference<int&>::type 结果为 int.因为还有个...::type& ,forward 函数的参数类型为 int&.
// 3.返回类型为 T&&即 int& &&,根据引用折叠规则,int& && 折叠为 int&

forward<int&&>(10); // R
// 1.模板参数推导 T 被推导为 int,因为传递的是右值
// 2. ... forward 参数类型为int&
// 3.返回类型为 T&&,即 int&&


template <typename T>
typename std::remove_reference<T>::type&& move(T&& t) noexcept {
	using ReturnType = typename std::remove_reference<T>::type&&;
	return static_cast<ReturnType>(t); // 它会剥离引用属性,然后将结果类型转换为右值引用
}

int x = 10;
move(x); // L
// 1. T 被推导为 int&,因为 x 是一个左值引用
// 2. 剥离引用属性:std::remove_reference<int&>::type 结果为 int.因为还有个...::type&& ,ReturnType 被定义为 int&&
// 3. 返回: static_cast<int&&>(x) 强制将左值 x 转换为右值引用

// 传入的是一个右值 int&& 或者基础类型 int
move(10);          // 右值
move(std::move(x)); // 右值引用 R
// 1. 模板参数推导: T 被推导为 int,因为传递的是右值或右值引用
// 2. 剥离引用属性: std::remove_reference<int>::type 结果为 int.因为还有个...::type&& ReturnType 被定义为 int&&
// 3. 返回: static_cast<int&&>(x) 直接将右值 x 转换为右值引用
// 传入右值的 std::move 来说,返回的 static_cast<int&&>(t) 确实在表面上看起来没有实际效果,因为它是将 int&& 转换为 int&&.
// 但从实现细节和代码风格上,使用 static_cast 仍然是一个重要的步骤,以确保移动语义的明确性
#endif