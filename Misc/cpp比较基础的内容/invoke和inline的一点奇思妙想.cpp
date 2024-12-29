//
// Created by fzhinvg on 2024/12/29.
// 一切看编译器的具体实现
#include <iostream>
#include <functional>
#include <vector>

struct Foo
{
	int value;

	explicit Foo(int v) : value(v)
	{}

	void print() const
	{
		std::cout << "Foo::print() - value: " << value << std::endl;
	}

	[[nodiscard]] int add(int a) const
	{
		return value + a;
	}
};

void globalFunction(int a)
{
	std::cout << "Global function called with: " << a << std::endl;
}

inline int add(int a, int b)
{
	return a + b;
} // 所以inline充其量算是给编译器的一种建议，具体行为含得看编译器自己的决定

__attribute__((always_inline)) inline int add_always(int a, int b)
{
	return a + b;
} // gcc clang 强制使用内联嵌入的方式
// 强制内联可能导致代码膨胀,即代码体积增大,这可能会对程序的性能产生负面影响 哈哈

int main()
{
	// 使用 std::invoke 调用全局函数
	std::invoke(globalFunction, 42);

	// 使用 std::invoke 调用成员函数
	Foo foo(10);
	std::invoke(&Foo::print, foo);
	//  ↓
	// 使用 std::invoke 调用成员函数并传递参数
	int result = std::invoke(&Foo::add, foo, 5);
	std::cout << "Result of Foo::add(5): " << result << std::endl;
	int result_compare = foo.add(5);
	// 但是我看invoke对应反汇编比这么直接调用复杂了好多啊
	// 我有点怀疑这么做除了统一调用风格之外到底有没有好处
	// 风格是好了点,但是也不是非常好,但是指令数量却更大了

	// 使用 std::invoke 调用 lambda 表达式
	auto lambda = [](int x)
	{
		std::cout << "Lambda called with: " << x << std::endl;
	};
	std::invoke(lambda, 7);

	// 使用 std::invoke 调用函数对象
	std::function<void(int)> func = globalFunction;
	std::invoke(func, 88);

	int x = 42;
	int y = 24;
	int result_inline = add(x, y);
//	mov	ecx, dword ptr [rbp - 40]
//	mov	edx, dword ptr [rbp - 44]
//	call	add(int, int)
//	mov	dword ptr [rbp - 76], eax
//	jmp	.LBB4_2
//			.LBB4_2:
//	mov	eax, dword ptr [rbp - 76]
//	mov	dword ptr [rbp - 48], eax
// 此处选择还是生成函数调用,而不是内联展开
	int result_inline_always = add_always(x, y);

	std::cout << "Result_inline: " << result << std::endl;

	return 0;
}

// 学习就是在不断打破之前的认知,每当打破自己原有认知的时候,说明我成长了
// 保持试错和纠错
// "Success is not final, failure is not fatal: It is the courage to continue that counts."