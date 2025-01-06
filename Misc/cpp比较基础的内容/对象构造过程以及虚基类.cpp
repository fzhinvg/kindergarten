//
// Created by fzhinvg on 2025/1/6.
//
#include <iostream>
/*
 * 对象初始化过程
 * 1.虚基类
 * 2.各级基类
 * 3.成员初始化列表
 * 4.对象自身构造器
 * */
// 这种构造过程的设计思路就像建筑师一样精妙绝伦
// 虚基类：为了避免菱形继承中出现多个基类实例，这里是class A
class A
{
public:
	A()
	{
		std::cout << "A constructor called" << std::endl;
	}
};

class B : virtual public A // 删除这里的virtual以解除虚基类
{
public:
	B()
	{
		std::cout << "B constructor called" << std::endl;
	}
};

class C : virtual public A // 删除这里的virtual以解除虚基类
{
public:
	C()
	{
		std::cout << "C constructor called" << std::endl;
	}
};

class D : public B, public C
{
public:
	D()
	{
		std::cout << "D constructor called" << std::endl;
	}
};

int main()
{
	D obj;
	return 0;
}
//  A   A
//  |   |
//  B   C
//   \ /
//    D
//    ↓ 虚基类
//    A
//   / \
//  B   C
//   \ /
//    D

