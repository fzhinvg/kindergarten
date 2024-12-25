//
// Created by fzhinvg on 2024/12/26.
//
/*
|-----------------------|
|        栈区            |  -> 用于函数调用和局部变量
|-----------------------|
|        堆区            |  -> 用于动态内存分配
|-----------------------|
|   已初始化数据区(.data)  |  -> 存储已初始化的全局和静态变量
|-----------------------|
|   未初始化数据区(.bss)   |  -> 存储未初始化的全局和静态变量
|-----------------------|
|       代码区(.text)    |  -> 存储可执行指令
|-----------------------|
|     共享库区(DLLs)      |  -> 存储动态链接库
|-----------------------|
*/
// 大多数人仅仅讨论堆栈,可能是因为他们所需要处理的层级并不需要设计到其它区域
// 我想那些可以为os编写代码的工作人员应该对这个相当之熟悉
#include <iostream>
#include <new>

#define CHINESE true
#define ENGLISH false


// 全局变量（已初始化数据区 .data）
int globalInitializedVar = 42;

// 全局变量（未初始化数据区 .bss）
int globalUninitializedVar;

// 全局静态变量（已初始化数据区 .data）
static int globalStaticVar = 30;

// 全局静态变量（未初始化数据区 .bss）
static int globalStaticUninitializedVar;
//.bss 段在二进制文件中占用的空间非常少,仅仅包含符号信息,而不占用实际的磁盘空间
// 然而,在程序运行时,操作系统会为 .bss 段分配实际的内存

void demonstrateMemoryLayout()
{
	// 局部变量（栈区）
	int localVar = 10;

	// 动态分配的内存（堆区）
	int *heapVar = new int[10];

	// 静态局部变量（已初始化数据区 .data）
	static int staticLocalVar = 20;

	// 静态局部变量（未初始化数据区 .bss）
	static int staticUninitializedVar;

#if CHINESE
	// 显示各变量的地址
	std::cout << "代码区（函数地址）: " << (void *) demonstrateMemoryLayout << std::endl;
	// 程序的代码段和数据段需要加载到主存（RAM）中
	// CPU 使用三缓来加速内存访问,缓解 RAM 的速度限制
	// 操作系统使用分页,分段和预取机制来有效管理内存,提升性能
	std::cout << "已初始化数据区（全局变量）: " << &globalInitializedVar << std::endl;
	std::cout << "未初始化数据区（全局变量）: " << &globalUninitializedVar << std::endl;
	std::cout << "已初始化数据区（全局静态变量）: " << &globalStaticVar << std::endl;
	std::cout << "未初始化数据区（全局静态变量）: " << &globalStaticUninitializedVar << std::endl;
	std::cout << "栈区（局部变量）: " << &localVar << std::endl;
	std::cout << "堆区（动态分配内存）: " << heapVar << std::endl;
	std::cout << "已初始化数据区（静态局部变量）: " << &staticLocalVar << std::endl;
	std::cout << "未初始化数据区（静态局部变量）: " << &staticUninitializedVar << std::endl;
#endif
#if ENGLISH // 因为控制台输出问题
	std::cout << "Code segment (function address): " << (void *) demonstrateMemoryLayout << std::endl;
	std::cout << "Initialized data segment (global variable): " << &globalInitializedVar << std::endl;
	std::cout << "Uninitialized data segment (global variable): " << &globalUninitializedVar << std::endl;
	std::cout << "Initialized data segment (global static variable): " << &globalStaticVar << std::endl;
	std::cout << "Uninitialized data segment (global static variable): " << &globalStaticUninitializedVar << std::endl;
	std::cout << "Stack segment (local variable): " << &localVar << std::endl;
	std::cout << "Heap segment (dynamically allocated memory): " << heapVar << std::endl;
	std::cout << "Initialized data segment (static local variable): " << &staticLocalVar << std::endl;
	std::cout << "Uninitialized data segment (static local variable): " << &staticUninitializedVar << std::endl;
#endif
	// 释放动态分配的内存
	delete[] heapVar;
}

int main()
{
	demonstrateMemoryLayout();
	return 0;
}

