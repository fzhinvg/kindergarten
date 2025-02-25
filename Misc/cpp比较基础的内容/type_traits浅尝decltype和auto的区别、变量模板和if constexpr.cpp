//
// Created by fzhinvg on 2024/12/29.
// 本文件相当杂乱
#include <iostream>
#include <type_traits>

int foo(int a, double b)
{
	return static_cast<int>(a + b);
}

using FuncPtr = decltype(&foo);

template<typename T>
void process(T value)
{
	// if constexpr实际上的作用是把它的作用域内的代码进行条件编译,完全没有if语句判断和分支的功能
	// 我更应该把它视作一个条件编译代码块的工具,而不是分支工具
	if constexpr (std::is_integral<T>::value)
	{
		std::cout << "Integral type: " << value << std::endl;
	} else if constexpr (std::is_floating_point<T>::value)
	{
		std::cout << "Floating point type: " << value << std::endl;
	} else
	{
		std::cout << "Other type" << std::endl;
	}
}

template<typename T>
void extendedProcess(T value)
{
	if constexpr (std::is_integral<T>::value)
	{
		std::cout << "Integral type: " << value << std::endl;
	} else if constexpr (std::is_floating_point<T>::value)
	{
		std::cout << "Floating point type: " << value << std::endl;
	} else if constexpr (std::is_pointer<T>::value)
	{
		std::cout << "Pointer type: " << *value << std::endl;
	} else if constexpr (std::is_array<T>::value)
	{
		std::cout << "Array type" << std::endl;
	} else if constexpr (std::is_const<T>::value)
	{
		std::cout << "Const type" << std::endl;
	} else
	{
		std::cout << "Other type" << std::endl;
	}
}

// auto 和 decltype 示例函数
void autoDecltypeDemo()
{
	int i = 42;
	const int &ref = i;

	// auto 在类型推导时会忽略顶层的 const 和 volatile 属性
	auto x = ref; // x 的类型是 int,auto 会忽略顶层的 const 和引用
	auto y = &i;  // y 的类型是 int*,指针类型
	auto z = ref; // z 的类型是 int,auto 会忽略顶层的 const 和引用

	// decltype 会保留表达式的所有类型信息,包括 const 和 volatile 属性
	decltype(ref) a = i; // a 的类型是 const int&,保持 const 和引用
	decltype((i)) b = i; // b 的类型是 int&,这里 (i) 是一个左值表达式,所以类型是 int&
	decltype(i) c = i;   // c 的类型是 int

	std::cout << "auto x: " << x << std::endl;
	std::cout << "auto y: " << *y << std::endl;
	std::cout << "auto z: " << z << std::endl;
	std::cout << "decltype a: " << a << std::endl;
	std::cout << "decltype b: " << b << std::endl;
	std::cout << "decltype c: " << c << std::endl;
}

void decltypeDemo()
{
	int a = 5;
	double b = 3.14;
	auto c = a + b;

	decltype(a) x = 10;  // x 类型为 int
	decltype(b) y = 2.71; // y 类型为 double
	decltype(c) z = 7.5; // z 类型为 double

	std::cout << "x: " << x << ", y: " << y << ", z: " << z << std::endl;
}

template<typename T>
constexpr T pi = T(3.1415926535897932385);

void variableTemplateDemo()
{
	std::cout << "pi<int>: " << pi<int> << std::endl;
	std::cout << "pi<float>: " << pi<float> << std::endl;
	std::cout << "pi<double>: " << pi<double> << std::endl;
}

struct Foo
{
	int add(int a, int b)
	{
		return a + b;
	}
};

// discard 已遗弃
// std::invoke 示例函数
//void invokeDemo()
//{
//	Foo foo;
//	auto memFn = &Foo::add;
//	std::cout << "Result of member function invoke: " << std::invoke(memFn, foo, 1, 2) << std::endl;
//
//	auto lambda = [](int x, int y) { return x * y; };
//	std::cout << "Result of lambda invoke: " << std::invoke(lambda, 3, 4) << std::endl;
//}

int main()
{
	// 定义一个函数指针并指向 foo
	FuncPtr ptr = &foo;
//	FuncPtr ptr = foo; // 等价

	// 使用函数指针调用函数
	int result = ptr(5, 3.2);
	std::cout << "Result: " << result << std::endl;

	process(42);          // 整数类型
//	process(3.14);        // 浮点类型 把这行注释会发现对应的float输出代码并没有对应特化反汇编
	process("Hello");     // 字符串（其他类型）
//	process('a');     // int type

	variableTemplateDemo();

//	invokeDemo(); // 已遗弃


	return 0;
}
