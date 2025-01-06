//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>

class Base
{
public:
	virtual void regularFunction() const
	{
		std::cout << "Base regular function implementation." << std::endl;
	}

	virtual ~Base() = default;
};

class Derived : public Base
{
public:
	void regularFunction() const override
	{
		std::cout << "Derived regular function implementation." << std::endl;
	}

	int get_pData() const
	{
		return 43;
	}

	static int staticFunc()
	{
		std::cout << "Derived static function implementation." << std::endl;
		return 42;
	}
};

int main()
{
	// 定义成员函数指针
	using FuncPtr = int (Derived::*)() const;

	// 获取成员函数指针
	FuncPtr funcPtr = &Derived::get_pData;

	// 创建一个虚假的对象地址
	auto fakeObject = reinterpret_cast<Derived *>(0xDEADBEEF);

	// 调用成员函数指针
	std::cout << (fakeObject->*funcPtr)() << std::endl;

//	// 调用静态成员函数
//	using StaticFuncPtr = int (*)();
//	StaticFuncPtr staticFuncPtr = reinterpret_cast<StaticFuncPtr>(&Derived::staticFunc);
//	std::cout << staticFuncPtr() << std::endl; // 我当初写这个东西是为了干嘛?怎么有种脱裤子放屁的感觉

	return 0;
}
