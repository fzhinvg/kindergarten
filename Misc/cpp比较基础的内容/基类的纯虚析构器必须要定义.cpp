//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>

class Base
{
public:
	virtual ~Base() = 0; // 纯虚析构函数的声明
};

Base::~Base()
{
	// 纯虚析构函数的定义
	std::cout << "Base destructor called" << std::endl;
}

// 如果没有提供纯虚析构函数的定义，在销毁派生类对象时，编译器无法找到基类析构函数的实现，导致链接错误
class Derived : Base
{
public:
	~Derived() override
	{ std::cout << "Derived destructor called" << std::endl; }
};


int main()
{
	auto derived = new Derived;
	delete derived;
	return 0;
}
